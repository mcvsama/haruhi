/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
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
#include <Qt3Support/Q3GroupBox>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/controller_proxy.h>
#include <haruhi/widgets/envelope_plot.h>

// Local:
#include "mikuru.h"
#include "envelopes.h"
#include "envelopes_adsr.h"
#include "voice.h"
#include "voice_manager.h"
#include "part.h"


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
	create_proxies();
	create_knobs (knobs_panel);
	create_widgets (knobs_panel);
	update_plot();
}


ADSR::~ADSR()
{
	_plot->assign_envelope (0);
	_mikuru->free_id ("adsrs", _id);

	// Delete knobs before ControllerProxies:
	delete _control_delay;
	delete _control_attack;
	delete _control_attack_hold;
	delete _control_decay;
	delete _control_sustain;
	delete _control_sustain_hold;
	delete _control_release;

	delete _proxy_delay;
	delete _proxy_attack;
	delete _proxy_attack_hold;
	delete _proxy_decay;
	delete _proxy_sustain;
	delete _proxy_sustain_hold;
	delete _proxy_release;

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
	_mikuru->graph()->unlock();
}


void
ADSR::create_ports()
{
	_mikuru->graph()->lock();
	_port_group = new Core::PortGroup (_mikuru->graph(), QString ("ADSR %1").arg (this->id()).toStdString());
	// Inputs:
	_port_delay = new Core::EventPort (_mikuru, "Delay", Core::Port::Input, _port_group);
	_port_attack = new Core::EventPort (_mikuru, "Attack", Core::Port::Input, _port_group);
	_port_attack_hold = new Core::EventPort (_mikuru, "Attack hold", Core::Port::Input, _port_group);
	_port_decay = new Core::EventPort (_mikuru, "Decay", Core::Port::Input, _port_group);
	_port_sustain = new Core::EventPort (_mikuru, "Sustain", Core::Port::Input, _port_group);
	_port_sustain_hold = new Core::EventPort (_mikuru, "Sustain hold", Core::Port::Input, _port_group);
	_port_release = new Core::EventPort (_mikuru, "Release", Core::Port::Input, _port_group);
	// Outputs:
	_port_output = new Core::EventPort (_mikuru, QString ("ADSR %1").arg (this->id()).toStdString(), Core::Port::Output, 0, Core::Port::Polyphonic);
	_mikuru->graph()->unlock();
}


void
ADSR::create_proxies()
{
	Params::ADSR p = _params;

	_proxy_delay = new ControllerProxy (_port_delay, &_params.delay, 0, HARUHI_MIKURU_MINMAX (Params::ADSR::Delay), p.delay);
	_proxy_delay->config()->curve = 1.0;
	_proxy_delay->apply_config();
	_proxy_attack = new ControllerProxy (_port_attack, &_params.attack, 0, HARUHI_MIKURU_MINMAX (Params::ADSR::Attack), p.attack);
	_proxy_attack->config()->curve = 1.0;
	_proxy_attack->apply_config();
	_proxy_attack_hold = new ControllerProxy (_port_attack_hold, &_params.attack_hold, 0, HARUHI_MIKURU_MINMAX (Params::ADSR::AttackHold), p.attack_hold);
	_proxy_attack_hold->config()->curve = 1.0;
	_proxy_attack_hold->apply_config();
	_proxy_decay = new ControllerProxy (_port_decay, &_params.decay, 0, HARUHI_MIKURU_MINMAX (Params::ADSR::Decay), p.decay);
	_proxy_decay->config()->curve = 1.0;
	_proxy_decay->apply_config();
	_proxy_sustain = new ControllerProxy (_port_sustain, &_params.sustain, 0, HARUHI_MIKURU_MINMAX (Params::ADSR::Sustain), p.sustain);
	_proxy_sustain_hold = new ControllerProxy (_port_sustain_hold, &_params.sustain_hold, 0, HARUHI_MIKURU_MINMAX (Params::ADSR::SustainHold), p.sustain_hold);
	_proxy_sustain_hold->config()->curve = 1.0;
	_proxy_sustain_hold->apply_config();
	_proxy_release = new ControllerProxy (_port_release, &_params.release, 0, HARUHI_MIKURU_MINMAX (Params::ADSR::Release), p.release);
	_proxy_release->config()->curve = 1.0;
	_proxy_release->apply_config();
}


void
ADSR::create_knobs (QWidget* parent)
{
	_control_delay = new Knob (parent, _proxy_delay, "Delay", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::Delay, 100), 2);
	_control_delay->set_unit_bay (_mikuru->unit_bay());
	_control_attack = new Knob (parent, _proxy_attack, "Attack", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::Attack, 100), 2);
	_control_attack->set_unit_bay (_mikuru->unit_bay());
	_control_attack_hold = new Knob (parent, _proxy_attack_hold, "Att.hold", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::AttackHold, 100), 2);
	_control_attack_hold->set_unit_bay (_mikuru->unit_bay());
	_control_decay = new Knob (parent, _proxy_decay, "Decay", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::Decay, 100), 2);
	_control_decay->set_unit_bay (_mikuru->unit_bay());
	_control_sustain = new Knob (parent, _proxy_sustain, "Sustain", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::Sustain, 100), 2);
	_control_sustain->set_unit_bay (_mikuru->unit_bay());
	_control_sustain_hold = new Knob (parent, _proxy_sustain_hold, "Sus.hold", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::SustainHold, 100), 2);
	_control_sustain_hold->set_unit_bay (_mikuru->unit_bay());
	_control_release = new Knob (parent, _proxy_release, "Release", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::ADSR::Release, 100), 2);
	_control_release->set_unit_bay (_mikuru->unit_bay());

	QObject::connect (_control_delay, SIGNAL (changed (int)), this, SLOT (update_plot()));
	QObject::connect (_control_attack, SIGNAL (changed (int)), this, SLOT (update_plot()));
	QObject::connect (_control_attack_hold, SIGNAL (changed (int)), this, SLOT (update_plot()));
	QObject::connect (_control_decay, SIGNAL (changed (int)), this, SLOT (update_plot()));
	QObject::connect (_control_sustain, SIGNAL (changed (int)), this, SLOT (update_plot()));
	QObject::connect (_control_sustain_hold, SIGNAL (changed (int)), this, SLOT (update_plot()));
	QObject::connect (_control_release, SIGNAL (changed (int)), this, SLOT (update_plot()));
}


void
ADSR::create_widgets (QWidget* knobs_panel)
{
	Params::ADSR p = _params;

	QFrame* plot_frame = new QFrame (knobs_panel);
	plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_plot = new EnvelopePlot (plot_frame);
	QVBoxLayout* plot_frame_layout = new QVBoxLayout (plot_frame, 0, Config::spacing);
	plot_frame_layout->addWidget (_plot);

	Q3GroupBox* grid1 = new Q3GroupBox (2, Qt::Horizontal, "", this);
	grid1->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	grid1->setInsideMargin (3 * Config::margin);

	_enabled = new QCheckBox ("Enabled", grid1);
	_enabled->setChecked (p.enabled);
	QObject::connect (_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	_direct_adsr = new QCheckBox ("Main ADSR", grid1);
	_direct_adsr->setChecked (p.direct_adsr);
	QObject::connect (_direct_adsr, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QToolTip::add (_direct_adsr, "Act as all parts main ADSR");

	_forced_release = new QCheckBox ("Forced Release", grid1);
	_forced_release->setChecked (p.forced_release);
	QObject::connect (_forced_release, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QToolTip::add (_forced_release, "Starts releasing even when no sustain point has been reached");

	_sustain_enabled = new QCheckBox ("Sustain", grid1);
	_sustain_enabled->setChecked (p.sustain_enabled);
	QObject::connect (_sustain_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QToolTip::add (_sustain_enabled, "Sustaining enabled");

	new QLabel ("Function:", grid1);
	_function = new QComboBox (grid1);
	_function->insertItem ("Log. E", Params::ADSR::LogarithmicE);
	_function->insertItem ("Log. 2", Params::ADSR::Logarithmic2);
	_function->insertItem ("Linear", Params::ADSR::Linear);
	_function->insertItem ("Exp. 2", Params::ADSR::Expotential2);
	_function->insertItem ("Exp. E", Params::ADSR::ExpotentialE);
	_function->setCurrentItem (Params::ADSR::Linear);
	QObject::connect (_function, SIGNAL (activated (int)), this, SLOT (update_params()));

	new QLabel ("Mode:", grid1);
	_mode = new QComboBox (grid1);
	_mode->insertItem ("Polyphonic", Params::ADSR::Polyphonic);
	_mode->insertItem ("Common Keysync", Params::ADSR::CommonKeySync);
	_mode->setCurrentItem (Params::ADSR::Polyphonic);
	QObject::connect (_mode, SIGNAL (activated (int)), this, SLOT (update_params()));

	QVBoxLayout* v1 = new QVBoxLayout (knobs_panel, 0, Config::spacing);
	QHBoxLayout* h1 = new QHBoxLayout (v1, Config::spacing);
	h1->addWidget (_control_delay);
	h1->addWidget (_control_attack);
	h1->addWidget (_control_decay);
	h1->addWidget (_control_sustain);
	h1->addWidget (_control_release);
	QHBoxLayout* h2 = new QHBoxLayout (v1, Config::spacing);
	h2->addWidget (_control_attack_hold);
	h2->addWidget (_control_sustain_hold);
	h2->addWidget (plot_frame);
	v1->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	QHBoxLayout* h3 = new QHBoxLayout (this, Config::margin, Config::spacing);
	h3->addWidget (knobs_panel);
	QVBoxLayout* v2 = new QVBoxLayout (h3, Config::spacing);
	v2->addWidget (grid1);
	v2->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}


void
ADSR::voice_created (VoiceManager* voice_manager, Voice* voice)
{
	if (!atomic (_params.enabled))
		return;

	uint64_t sr = _mikuru->graph()->sample_rate();
	// Minimum attack/release time (ms), prevents clicking:
	uint64_t min = 0.005f * sr;
	_adsrs[voice] = new DSP::ADSR (
		sr * atomic (_params.delay) / Params::ADSR::DelayDenominator,
		std::max (sr * atomic (_params.attack) / Params::ADSR::AttackDenominator, min),
		sr * atomic (_params.attack_hold) / Params::ADSR::AttackHoldDenominator,
		sr * atomic (_params.decay) / Params::ADSR::DecayDenominator,
		1.0f * atomic (_params.sustain) / Params::ADSR::SustainDenominator,
		sr * atomic (_params.sustain_hold) / Params::ADSR::SustainHoldDenominator,
		std::max (sr * atomic (_params.release) / Params::ADSR::ReleaseDenominator, min),
		_params.sustain_enabled,
		_params.forced_release
	);
	if (atomic (_params.direct_adsr))
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
	_proxy_delay->process_events();
	_proxy_attack->process_events();
	_proxy_attack_hold->process_events();
	_proxy_decay->process_events();
	_proxy_sustain->process_events();
	_proxy_sustain_hold->process_events();
	_proxy_release->process_events();

	if (_adsrs.empty())
		return;

	Core::Timestamp t = _mikuru->graph()->timestamp();
	Core::Sample v;
	const bool direct_adsr = atomic (_params.direct_adsr);

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
			case Params::ADSR::LogarithmicE: v = std::pow (v, 1.0f/M_E);	break;
			case Params::ADSR::Logarithmic2: v = std::pow (v, 0.5f);		break;
			case Params::ADSR::Linear:										break;
			case Params::ADSR::Expotential2: v = std::pow (v, 2.0f);		break;
			case Params::ADSR::ExpotentialE: v = std::pow (v, M_E);			break;
		}

		if (adsr->finished())
		{
			// All-parts direct adsr modulation:
			if (direct_adsr)
				voice->voice_manager()->set_voice_param (voice->voice_id(), &Params::Voice::adsr, Params::Voice::AdsrMin);
			// Mute sound completely:
			_port_output->event_buffer()->push (new Core::VoiceControllerEvent (t, voice->voice_id(), 0.0f));
			// Don't call VoiceManager#voice_event() directly, because it will callback our methods. Use buffer.
			if (direct_adsr)
				voice->voice_manager()->buffer_voice_event (new Core::VoiceEvent (t, Core::OmniKey, voice->voice_id(), Core::VoiceEvent::Drop, 0.0, 0.0));
		}
		else
		{
			// All-parts direct adsr modulation:
			if (direct_adsr)
				voice->voice_manager()->set_voice_param (voice->voice_id(), &Params::Voice::adsr, Params::Voice::AdsrMax * v);
			// Normal event on output:
			_port_output->event_buffer()->push (new Core::VoiceControllerEvent (t, voice->voice_id(), v));
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

	_proxy_delay->set_value (p.delay);
	_proxy_attack->set_value (p.attack);
	_proxy_attack_hold->set_value (p.attack_hold);
	_proxy_decay->set_value (p.decay);
	_proxy_sustain->set_value (p.sustain);
	_proxy_sustain_hold->set_value (p.sustain_hold);
	_proxy_release->set_value (p.release);

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

	atomic (_params.enabled) = _enabled->isChecked();
	atomic (_params.direct_adsr) = _direct_adsr->isChecked();
	atomic (_params.forced_release) = _forced_release->isChecked();
	atomic (_params.sustain_enabled) = _sustain_enabled->isChecked();
	atomic (_params.function) = _function->currentItem();
	atomic (_params.mode) = _mode->currentItem();

	// Knob params are updated automatically using #assign_parameter.
}


void
ADSR::update_plot()
{
	// If graph was not connected, sample rate would be 0 which would cause plotting problems.
	uint64_t sr = std::max (1000u, _mikuru->graph()->sample_rate());

	float sustain_value = 1.0f * atomic (_params.sustain) / Params::ADSR::SustainDenominator;
	DSP::Envelope::Points& points = _envelope_for_plot.points();
	points.clear();
	points.push_back (DSP::Envelope::Point (0.0f, sr * atomic (_params.delay) / Params::ADSR::DelayDenominator));
	points.push_back (DSP::Envelope::Point (0.0f, sr * atomic (_params.attack) / Params::ADSR::AttackDenominator));
	points.push_back (DSP::Envelope::Point (1.0f, sr * atomic (_params.attack_hold) / Params::ADSR::AttackHoldDenominator));
	points.push_back (DSP::Envelope::Point (1.0f, sr * atomic (_params.decay) / Params::ADSR::DecayDenominator));
	points.push_back (DSP::Envelope::Point (sustain_value, sr * atomic (_params.sustain_hold) / Params::ADSR::SustainHoldDenominator));
	points.push_back (DSP::Envelope::Point (sustain_value, sr * atomic (_params.release) / Params::ADSR::ReleaseDenominator));
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

