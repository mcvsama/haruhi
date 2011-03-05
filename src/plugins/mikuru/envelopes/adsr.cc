/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>
#include <algorithm>
#include <stdint.h>
#include <cmath>
#include <set>

// Qt:
#include <QtGui/QToolTip>
#include <QtGui/QGroupBox>
#include <QtGui/QLayout>
#include <QtGui/QGridLayout>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/widgets/envelope_plot.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "../mikuru.h"
#include "../envelopes.h"
#include "../voice.h"
#include "../voice_manager.h"
#include "../part.h"
#include "adsr.h"


namespace MikuruPrivate {

ADSR::ADSR (int id, Mikuru* mikuru, QWidget* parent):
	Envelope (parent),
	_mikuru (mikuru),
	_loading_params (false),
	_buffer (mikuru->graph()->buffer_size())
{
	_id = (id == 0) ? _mikuru->allocate_id ("adsrs") : _mikuru->reserve_id ("adsrs", id);

	QWidget* knobs_panel = new QWidget (this);
	knobs_panel->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Expanding);

	create_ports();
	create_knobs (knobs_panel);
	create_widgets (knobs_panel);
	update_params();
	update_plot();
}


ADSR::~ADSR()
{
	_plot->assign_envelope (0);
	_mikuru->free_id ("adsrs", _id);

	delete _knob_delay;
	delete _knob_attack;
	delete _knob_attack_hold;
	delete _knob_decay;
	delete _knob_sustain;
	delete _knob_sustain_hold;
	delete _knob_release;

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	delete _port_output;
	delete _port_delay;
	delete _port_attack;
	delete _port_attack_hold;
	delete _port_decay;
	delete _port_sustain;
	delete _port_sustain_hold;
	delete _port_release;
	delete _port_group;

	sweep();
	// Delete remaining ADSRs:
	for (ADSRs::iterator x = _adsrs.begin(); x != _adsrs.end(); ++x)
	{
		x->first->set_tracked (false);
		delete x->second;
	}
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
ADSR::create_ports()
{
	if (_mikuru->graph())
		_mikuru->graph()->lock();
	_port_group = new Haruhi::PortGroup (_mikuru->graph(), QString ("ADSR %1").arg (this->id()).toStdString());
	// Inputs:
	_port_delay			= new Haruhi::EventPort (_mikuru, "Delay", Haruhi::Port::Input, _port_group);
	_port_attack		= new Haruhi::EventPort (_mikuru, "Attack", Haruhi::Port::Input, _port_group);
	_port_attack_hold	= new Haruhi::EventPort (_mikuru, "Attack hold", Haruhi::Port::Input, _port_group);
	_port_decay			= new Haruhi::EventPort (_mikuru, "Decay", Haruhi::Port::Input, _port_group);
	_port_sustain		= new Haruhi::EventPort (_mikuru, "Sustain", Haruhi::Port::Input, _port_group);
	_port_sustain_hold	= new Haruhi::EventPort (_mikuru, "Sustain hold", Haruhi::Port::Input, _port_group);
	_port_release		= new Haruhi::EventPort (_mikuru, "Release", Haruhi::Port::Input, _port_group);
	// Outputs:
	_port_output = new Haruhi::EventPort (_mikuru, QString ("ADSR %1").arg (this->id()).toStdString(), Haruhi::Port::Output, 0, Haruhi::Port::Polyphonic);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
ADSR::create_knobs (QWidget* parent)
{
	_knob_delay			= new Haruhi::Knob (parent, _port_delay, &_params.delay, "Delay", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::Delay, 100), 2);
	_knob_attack		= new Haruhi::Knob (parent, _port_attack, &_params.attack, "Attack", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::Attack, 100), 2);
	_knob_attack_hold	= new Haruhi::Knob (parent, _port_attack_hold, &_params.attack_hold, "Att.hold", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::AttackHold, 100), 2);
	_knob_decay			= new Haruhi::Knob (parent, _port_decay, &_params.decay, "Decay", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::Decay, 100), 2);
	_knob_sustain		= new Haruhi::Knob (parent, _port_sustain, &_params.sustain, "Sustain", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::Sustain, 100), 2);
	_knob_sustain_hold	= new Haruhi::Knob (parent, _port_sustain_hold, &_params.sustain_hold, "Sus.hold", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::SustainHold, 100), 2);
	_knob_release		= new Haruhi::Knob (parent, _port_release, &_params.release, "Release", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::Release, 100), 2);

	// Some knobs need curve setup:
	Haruhi::Knob* curved_knobs[] = {
		_knob_delay, _knob_attack, _knob_attack_hold,
		_knob_decay, _knob_sustain_hold, _knob_release
	};
	for (Haruhi::Knob** k = curved_knobs; k != curved_knobs + sizeof (curved_knobs) / sizeof (*curved_knobs); ++k)
	{
		(*k)->controller_proxy().config().curve = 1.0;
		(*k)->controller_proxy().apply_config();
	}

	// All knobs have set unit_bay:
	Haruhi::Knob* all_knobs[] = {
		_knob_delay, _knob_attack, _knob_attack_hold,
		_knob_decay, _knob_sustain, _knob_sustain_hold, _knob_release
	};
	for (Haruhi::Knob** k = all_knobs; k != all_knobs + sizeof (all_knobs) / sizeof (*all_knobs); ++k)
	{
		(*k)->set_unit_bay (_mikuru->unit_bay());
		QObject::connect (*k, SIGNAL (changed (int)), this, SLOT (update_plot()));
	}
}


void
ADSR::create_widgets (QWidget* knobs_panel)
{
	Params::ADSR p = _params;

	QFrame* plot_frame = new QFrame (knobs_panel);
	plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_plot = new Haruhi::EnvelopePlot (plot_frame);

	QGroupBox* grid1 = new QGroupBox (this);
	QGridLayout* grid1_layout = new QGridLayout (grid1);
	grid1->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Maximum);
	grid1_layout->setMargin (3 * Config::Margin);

	_enabled = new QCheckBox ("Enabled", grid1);
	_enabled->setChecked (p.enabled);
	grid1_layout->addWidget (_enabled, 0, 0);
	QObject::connect (_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	_direct_adsr = new QCheckBox ("Main ADSR", grid1);
	_direct_adsr->setChecked (p.direct_adsr);
	grid1_layout->addWidget (_direct_adsr, 0, 1);
	QObject::connect (_direct_adsr, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QToolTip::add (_direct_adsr, "Act as all parts main ADSR");

	_forced_release = new QCheckBox ("Forced Release", grid1);
	_forced_release->setChecked (p.forced_release);
	grid1_layout->addWidget (_forced_release, 1, 0);
	QObject::connect (_forced_release, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QToolTip::add (_forced_release, "Starts releasing even when no sustain point has been reached");

	_sustain_enabled = new QCheckBox ("Sustain", grid1);
	_sustain_enabled->setChecked (p.sustain_enabled);
	grid1_layout->addWidget (_sustain_enabled, 1, 1);
	QObject::connect (_sustain_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QToolTip::add (_sustain_enabled, "Sustaining enabled");

	_function = new QComboBox (grid1);
	_function->insertItem ("Log. E", Params::ADSR::LogarithmicE);
	_function->insertItem ("Log. 2", Params::ADSR::Logarithmic2);
	_function->insertItem ("Linear", Params::ADSR::Linear);
	_function->insertItem ("Exp. 2", Params::ADSR::Expotential2);
	_function->insertItem ("Exp. E", Params::ADSR::ExpotentialE);
	_function->setCurrentItem (Params::ADSR::Linear);
	grid1_layout->addWidget (_function, 2, 0);
	QObject::connect (_function, SIGNAL (activated (int)), this, SLOT (update_params()));

	_mode = new QComboBox (grid1);
	_mode->insertItem ("Polyphonic", Params::ADSR::Polyphonic);
	_mode->insertItem ("Common Keysync", Params::ADSR::CommonKeySync);
	_mode->setCurrentItem (Params::ADSR::Polyphonic);
	grid1_layout->addWidget (_mode, 2, 1);
	QObject::connect (_mode, SIGNAL (activated (int)), this, SLOT (update_params()));

	// Layouts:

	QVBoxLayout* plot_frame_layout = new QVBoxLayout (plot_frame);
	plot_frame_layout->setMargin (0);
	plot_frame_layout->setSpacing (Config::Spacing);
	plot_frame_layout->addWidget (_plot);

	QHBoxLayout* h1 = new QHBoxLayout();
	h1->setSpacing (Config::Spacing);
	h1->addWidget (_knob_delay);
	h1->addWidget (_knob_attack);
	h1->addWidget (_knob_decay);
	h1->addWidget (_knob_sustain);
	h1->addWidget (_knob_release);

	QHBoxLayout* h2 = new QHBoxLayout();
	h2->setSpacing (Config::Spacing);
	h2->addWidget (_knob_attack_hold);
	h2->addWidget (_knob_sustain_hold);
	h2->addWidget (plot_frame);

	QVBoxLayout* v1 = new QVBoxLayout (knobs_panel);
	v1->setMargin (0);
	v1->setSpacing (Config::Spacing);
	v1->addLayout (h1);
	v1->addLayout (h2);
	v1->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	QVBoxLayout* v2 = new QVBoxLayout();
	v2->setSpacing (Config::Spacing);
	v2->addWidget (grid1);
	v2->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	QHBoxLayout* h3 = new QHBoxLayout (this);
	h3->setMargin (Config::Margin);
	h3->setSpacing (Config::Spacing);
	h3->addWidget (knobs_panel);
	h3->addLayout (v2);
}


void
ADSR::voice_created (VoiceManager* voice_manager, Voice* voice)
{
	if (!_params.enabled.get())
		return;

	uint64_t sr = _mikuru->graph()->sample_rate();
	// Minimum attack/release time (ms), prevents clicking:
	uint64_t min = 0.005f * sr;
	_adsrs[voice] = new DSP::ADSR (
		sr * _params.delay.to_f(),
		std::max (static_cast<uint64_t> (sr * _params.attack.to_f()), min),
		sr * _params.attack_hold.to_f(),
		sr * _params.decay.to_f(),
		_params.sustain.to_f(),
		sr * _params.sustain_hold.to_f(),
		std::max (static_cast<uint64_t> (sr * _params.release.to_f()), min),
		_params.sustain_enabled,
		_params.forced_release
	);
	if (_params.direct_adsr.get())
		voice->set_tracked (true);
}


void
ADSR::voice_released (VoiceManager* voice_manager, Voice* voice)
{
	ADSRs::iterator x = _adsrs.find (voice);
	if (x != _adsrs.end())
		x->second->release();
}


void
ADSR::voice_dropped (VoiceManager* voice_manager, Voice* voice)
{
	// Register ADSR to be deleted in next processing round,
	// because Voice may still be processed in this round.
	_dropped_voices.push_back (voice);
}


void
ADSR::process()
{
	sweep();

	// Sync input ports by hand, since we're before Mikuru's autosync:
	_port_delay->sync();
	_port_attack->sync();
	_port_attack_hold->sync();
	_port_decay->sync();
	_port_sustain->sync();
	_port_sustain_hold->sync();
	_port_release->sync();

	// Process ports events:
	_knob_delay->controller_proxy().process_events();
	_knob_attack->controller_proxy().process_events();
	_knob_attack_hold->controller_proxy().process_events();
	_knob_decay->controller_proxy().process_events();
	_knob_sustain->controller_proxy().process_events();
	_knob_sustain_hold->controller_proxy().process_events();
	_knob_release->controller_proxy().process_events();

	// Nothing to process?
	if (_adsrs.empty())
		return;

	Haruhi::Timestamp t = _mikuru->graph()->timestamp();
	Haruhi::Sample v;
	const bool direct_adsr = _params.direct_adsr.get();

	// Assuming that output ports are cleared by Mikuru on beginnig of each
	// processing round.

	for (ADSRs::iterator a = _adsrs.begin(); a != _adsrs.end(); ++a)
	{
		Voice* voice = a->first;
		DSP::ADSR* adsr = a->second;
		adsr->fill (_buffer.begin(), _buffer.end());
		v = *(_buffer.end() - 1);
		switch (_params.function)
		{
			case Params::ADSR::LogarithmicE: v = FastPow::pow (v, 1.0f/M_E);	break;
			case Params::ADSR::Logarithmic2: v = FastPow::pow (v, 0.5f);		break;
			case Params::ADSR::Linear:			            					break;
			case Params::ADSR::Expotential2: v = FastPow::pow (v, 2.0f);		break;
			case Params::ADSR::ExpotentialE: v = FastPow::pow (v, M_E);			break;
		}

		if (adsr->finished())
		{
			// All-parts direct adsr modulation:
			if (direct_adsr)
				voice->voice_manager()->set_voice_param (voice->voice_id(), &Params::Voice::adsr, Params::Voice::AdsrMin);
			// Mute sound completely:
			_port_output->event_buffer()->push (new Haruhi::VoiceControllerEvent (t, voice->voice_id(), 0.0f));
			// Don't call VoiceManager#voice_event() directly, because it will callback our methods. Use buffer.
			if (direct_adsr)
				voice->voice_manager()->buffer_voice_event (new Haruhi::VoiceEvent (t, Haruhi::OmniKey, voice->voice_id(), Haruhi::VoiceEvent::Drop, 0.0, 0.0));
		}
		else
		{
			// All-parts direct adsr modulation:
			if (direct_adsr)
				voice->voice_manager()->set_voice_param (voice->voice_id(), &Params::Voice::adsr, Params::Voice::AdsrMax * v);
			// Normal event on output:
			_port_output->event_buffer()->push (new Haruhi::VoiceControllerEvent (t, voice->voice_id(), v));
		}
	}
}


void
ADSR::resize_buffers (std::size_t size)
{
	_buffer.resize (size);
}


void
ADSR::load_params()
{
	// Copy params:
	Params::ADSR p (_params);
	_loading_params = true;

	_enabled->setChecked (p.enabled);
	_direct_adsr->setChecked (p.direct_adsr);
	_forced_release->setChecked (p.forced_release);
	_sustain_enabled->setChecked (p.sustain_enabled);
	_function->setCurrentItem (p.function);
	_mode->setCurrentItem (p.mode);

	_loading_params = false;
}


void
ADSR::load_params (Params::ADSR& params)
{
	_params = params;
	load_params();
}


void
ADSR::update_params()
{
	if (_loading_params)
		return;

	_params.enabled.set (_enabled->isChecked());
	_params.direct_adsr.set (_direct_adsr->isChecked());
	_params.forced_release.set (_forced_release->isChecked());
	_params.sustain_enabled.set (_sustain_enabled->isChecked());
	_params.function.set (_function->currentItem());
	_params.mode.set (_mode->currentItem());

	// Knob params are updated automatically using #assign_parameter.
}


void
ADSR::update_plot()
{
	// If graph was not connected, sample rate would be 0 which would cause plotting problems.
	uint64_t sr = std::max (1000u, _mikuru->graph()->sample_rate());

	float sustain_value = _params.sustain.to_f();
	DSP::Envelope::Points& points = _envelope_for_plot.points();
	points.clear();
	points.push_back (DSP::Envelope::Point (0.0f, sr * _params.delay.to_f()));
	points.push_back (DSP::Envelope::Point (0.0f, sr * _params.attack.to_f()));
	points.push_back (DSP::Envelope::Point (1.0f, sr * _params.attack_hold.to_f()));
	points.push_back (DSP::Envelope::Point (1.0f, sr * _params.decay.to_f()));
	points.push_back (DSP::Envelope::Point (sustain_value, sr * _params.sustain_hold.to_f()));
	points.push_back (DSP::Envelope::Point (sustain_value, sr * _params.release.to_f()));
	points.push_back (DSP::Envelope::Point (0.0f, 0));
	_envelope_for_plot.set_sustain_point (4);
	_plot->set_sample_rate (sr);
	_plot->assign_envelope (&_envelope_for_plot);
	_plot->plot_shape();
}


void
ADSR::sweep()
{
	for (std::list<Voice*>::iterator v = _dropped_voices.begin(); v != _dropped_voices.end(); ++v)
	{
		ADSRs::iterator a = _adsrs.find (*v);
		if (a != _adsrs.end())
		{
			delete a->second;
			_adsrs.erase (a);
		}
	}
	_dropped_voices.clear();
}

} // namespace MikuruPrivate

