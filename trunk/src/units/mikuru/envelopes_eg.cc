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
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/widgets/envelope_plot.h>

// Local: TODO sprawdź które include'y faktycznie potrzebne.
#include "mikuru.h"
#include "envelopes.h"
#include "envelopes_eg.h"
#include "voice.h"
#include "voice_manager.h"
#include "part.h"


namespace MikuruPrivate {

EG::EG (int id, Mikuru* mikuru, QWidget* parent):
	Envelope (parent),
	_mikuru (mikuru),
	_loading_params (false),
	_updating_widgets (false),
	_mute_point_controls (false),
	_buffer (mikuru->graph()->buffer_size()),
	_segment_duration (Params::EG::SegmentDurationMin, Params::EG::SegmentDurationMax,
					   Params::EG::SegmentDurationDefault, Params::EG::SegmentDurationDenominator),
	_point_value (Params::EG::PointValueMin, Params::EG::PointValueMax,
				  Params::EG::PointValueDefault, Params::EG::PointValueDenominator)
{
	_id = (id == 0) ? _mikuru->allocate_id ("egs") : _mikuru->reserve_id ("egs", id);

	_envelope_template.points().push_back (DSP::Envelope::Point (0.5, 0.5 * ARTIFICIAL_SAMPLE_RATE));
	_envelope_template.points().push_back (DSP::Envelope::Point (0.5, 0.5 * ARTIFICIAL_SAMPLE_RATE));
	_envelope_template.points().push_back (DSP::Envelope::Point (0.5, 0));

	create_ports();
	create_widgets();
	update_params();
	update_plot();
	update_widgets();
	update_point_knobs();
}


EG::~EG()
{
	_plot->assign_envelope (0);
	_mikuru->free_id ("egs", _id);

	// Delete knobs before ControllerProxies:
	delete _knob_point_value;
	delete _knob_segment_duration;

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	delete _port_point_value;
	delete _port_segment_duration;
	delete _port_output;

	sweep();
	// Delete remaining EGs:
	for (EGs::iterator x = _egs.begin(); x != _egs.end(); ++x)
	{
		x->first->set_tracked (false);
		delete x->second;
	}
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
EG::create_ports()
{
	if (_mikuru->graph())
		_mikuru->graph()->lock();
	_port_group = new Haruhi::PortGroup (_mikuru->graph(), QString ("EG %1").arg (this->id()).toStdString());
	// Inputs:
	_port_point_value = new Haruhi::EventPort (_mikuru, "Value", Haruhi::Port::Input, _port_group);
	_port_segment_duration = new Haruhi::EventPort (_mikuru, "Duration", Haruhi::Port::Input, _port_group);
	// Outputs:
	_port_output = new Haruhi::EventPort (_mikuru, QString ("EG %1").arg (this->id()).toStdString(), Haruhi::Port::Output, 0, Haruhi::Port::Polyphonic);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
EG::create_knobs (QWidget* parent)
{
	_knob_point_value = new Haruhi::Knob (parent, _port_point_value, &_point_value, "Value", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::EG::PointValue, 100), 2);
	_knob_point_value->set_unit_bay (_mikuru->unit_bay());

	_knob_segment_duration = new Haruhi::Knob (parent, _port_segment_duration, &_segment_duration, "Duration", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::EG::SegmentDuration, 100), 2);
	_knob_segment_duration->set_unit_bay (_mikuru->unit_bay());
	_knob_segment_duration->controller_proxy().config().curve = 1.0;
	_knob_segment_duration->controller_proxy().apply_config();

	QObject::connect (_knob_point_value, SIGNAL (changed (int)), this, SLOT (changed_segment_value()));
	QObject::connect (_knob_segment_duration, SIGNAL (changed (int)), this, SLOT (changed_segment_duration()));
}


void
EG::create_widgets()
{
	Params::EG p = _params;

	QWidget* knobs_panel = new QWidget (this);
	knobs_panel->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Expanding);

	create_knobs (knobs_panel);

	QFrame* plot_frame = new QFrame (knobs_panel);
	plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	plot_frame->setFixedWidth (200);
	plot_frame->setFixedHeight (100);
	_plot = new Haruhi::EnvelopePlot (plot_frame);
	_plot->set_editable (true, 1.0f * Params::EG::SegmentDurationMax / Params::EG::SegmentDurationDenominator);
	_plot->set_sample_rate (ARTIFICIAL_SAMPLE_RATE);
	_plot->assign_envelope (&_envelope_template);
	QObject::connect (_plot, SIGNAL (envelope_updated()), this, SLOT (changed_envelope()));
	QObject::connect (_plot, SIGNAL (active_point_changed()), this, SLOT (changed_envelope()));
	QVBoxLayout* plot_frame_layout = new QVBoxLayout (plot_frame, 0, Config::Spacing);
	plot_frame_layout->addWidget (_plot);

	_active_point = new QSpinBox (knobs_panel);
	_active_point->setMinimum (0);
	QObject::connect (_active_point, SIGNAL (valueChanged (int)), this, SLOT (changed_active_point()));

	QGroupBox* grid1 = new QGroupBox (this);
	QGridLayout* grid1_layout = new QGridLayout (grid1);
	grid1->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	grid1_layout->setMargin (3 * Config::Margin);

	_enabled = new QCheckBox ("Enabled", grid1);
	_enabled->setChecked (p.enabled);
	grid1_layout->addWidget (_enabled, 0, 0);
	QObject::connect (_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	grid1_layout->addWidget (new QLabel ("Sustain point:", grid1), 1, 0);
	_sustain_point = new QSpinBox (grid1);
	_sustain_point->setMinimum (1);
	grid1_layout->addWidget (_sustain_point, 1, 1);
	QObject::connect (_sustain_point, SIGNAL (valueChanged (int)), this, SLOT (update_widgets()));
	QObject::connect (_sustain_point, SIGNAL (valueChanged (int)), this, SLOT (update_params()));
	QObject::connect (_sustain_point, SIGNAL (valueChanged (int)), this, SLOT (update_plot()));

	grid1_layout->addWidget (new QLabel ("Segments:", grid1), 2, 0);
	_segments = new QSpinBox (grid1);
	_segments->setMinimum (2);
	_segments->setMaximum (Params::EG::MaxPoints - 1);
	grid1_layout->addWidget (_segments, 2, 1);
	QObject::connect (_segments, SIGNAL (valueChanged (int)), this, SLOT (update_widgets()));
	QObject::connect (_segments, SIGNAL (valueChanged (int)), this, SLOT (update_params()));
	QObject::connect (_segments, SIGNAL (valueChanged (int)), this, SLOT (update_plot()));

	QVBoxLayout* v1 = new QVBoxLayout (knobs_panel, 0, Config::Spacing);
	QHBoxLayout* h1 = new QHBoxLayout (v1, Config::Spacing);
	h1->addWidget (plot_frame);
	QHBoxLayout* h2 = new QHBoxLayout (v1, Config::Spacing);
	h2->addWidget (_knob_point_value);
	h2->addWidget (_knob_segment_duration);
	h2->addWidget (_active_point);
	v1->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	QHBoxLayout* h3 = new QHBoxLayout (this, Config::Margin, Config::Spacing);
	h3->addWidget (knobs_panel);
	QVBoxLayout* v2 = new QVBoxLayout (h3, Config::Spacing);
	v2->addWidget (grid1);
	v2->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}


void
EG::voice_created (VoiceManager* voice_manager, Voice* voice)
{
	if (!_params.enabled.get())
		return;

	unsigned int sample_rate = _mikuru->graph()->sample_rate();
	DSP::Envelope* env = _egs[voice] = new DSP::Envelope (_envelope_template);
	// Convert durations from ARTIFICIAL_SAMPLE_RATE to real sample rate:
	for (DSP::Envelope::Points::iterator p = env->points().begin(); p != env->points().end(); ++p)
		p->samples = 1.0f * p->samples / ARTIFICIAL_SAMPLE_RATE * sample_rate;
}


void
EG::voice_released (VoiceManager* voice_manager, Voice* voice)
{
	EGs::iterator x = _egs.find (voice);
	if (x != _egs.end())
		x->second->release();
}


void
EG::voice_dropped (VoiceManager* voice_manager, Voice* voice)
{
	// Register EG to be deleted in next processing round,
	// because Voice may still be processed in this round.
	_dropped_voices.push_back (voice);
}


void
EG::process()
{
	sweep();

	// Sync input ports by hand, since we're before Mikuru's autosync:
	_port_point_value->sync();
	_port_segment_duration->sync();

	// Process ports events:
	_knob_point_value->controller_proxy().process_events();
	_knob_segment_duration->controller_proxy().process_events();

	// Nothing to process?
	if (_egs.empty())
		return;

	Haruhi::Timestamp t = _mikuru->graph()->timestamp();
	Haruhi::Sample v;

	// Assuming that output ports are cleared by Mikuru on beginnig of each
	// processing round.

	for (EGs::iterator e = _egs.begin(); e != _egs.end(); ++e)
	{
		Voice* voice = e->first;
		DSP::Envelope* eg = e->second;
		eg->fill (_buffer.begin(), _buffer.end());
		v = *(_buffer.end() - 1);

		if (eg->finished())
		{
			// TODO ?
		}
		else
		{
			// Normal event on output:
			_port_output->event_buffer()->push (new Haruhi::VoiceControllerEvent (t, voice->voice_id(), v));
		}
	}
}


void
EG::resize_buffers (std::size_t size)
{
	_buffer.resize (size);
}


void
EG::load_params()
{
	// Copy params:
	Params::EG p (_params);
	_loading_params = true;

	_enabled->setChecked (p.enabled);
	_sustain_point->setValue (p.sustain_point + 1);
	_segments->setValue (p.segments);
	// Load points:
	_envelope_template.points().clear();
	for (size_t i = 0; i < p.segments + 1; ++i)
	{
		_envelope_template.points().push_back (
			DSP::Envelope::Point (1.0 * p.values[i] / Params::EG::PointValueDenominator, p.durations[i])
		);
	}

	_loading_params = false;

	// No knob is updated that would cause plot update,
	// do so manually:
	update_plot();
}


void
EG::load_params (Params::EG& params)
{
	_params = params;
	load_params();
}


void
EG::update_params()
{
	if (_loading_params)
		return;

	_params.enabled.set (_enabled->isChecked());
	_params.segments.set (_segments->value());
	_params.sustain_point.set (_sustain_point->value() - 1);

	// Update points:
	DSP::Envelope::Points& points = _envelope_template.points();
	size_t s = _params.segments.get() + 1;
	for (size_t i = 0; i < s; ++i)
	{
		_params.values[i].set (points[i].value * Params::EG::PointValueDenominator);
		_params.durations[i].set (points[i].samples);
	}

	// Knob params are updated automatically using #assign_parameter.

	_params.sanitize();
}


void
EG::update_plot()
{
	// If graph was not connected, sample rate would be 0 which would cause plotting problems.
	_envelope_template.set_sustain_point (_params.sustain_point);
	_plot->set_active_point (_active_point->value());

	DSP::Envelope::Points::size_type o = _envelope_template.points().size();
	_envelope_template.points().resize (_params.segments + 1);
	// Init value of new points to 0.5:
	for (unsigned int i = o; i < _envelope_template.points().size(); ++i)
		_envelope_template.points()[i].value = 0.5f;
	// Set length of the last segment to 0:
	_envelope_template.points().back().samples = 0;

	_plot->post_plot_shape();
}


void
EG::update_widgets()
{
	if (_updating_widgets)
		return;
	_updating_widgets = true;

	_sustain_point->setMaximum (_segments->value() + 1);
	_active_point->setMaximum (_segments->value());

	_updating_widgets = false;
}


void
EG::changed_active_point()
{
	_mute_point_controls = true;
	update_point_knobs();
	update_plot();
	_mute_point_controls = false;
}


void
EG::changed_segment_value()
{
	if (_mute_point_controls)
		return;

	unsigned int p = _active_point->value();
	_envelope_template.points()[p].value = _point_value.to_f();

	update_params();
	update_plot();
}


void
EG::changed_segment_duration()
{
	if (_mute_point_controls)
		return;

	unsigned int p = _active_point->value();
	if (p > 0)
		_envelope_template.points()[p-1].samples = _segment_duration.to_f() * ARTIFICIAL_SAMPLE_RATE;

	update_params();
	update_plot();
}


void
EG::changed_envelope()
{
	_mute_point_controls = true;
	update_point_knobs();
	_mute_point_controls = false;
	if (_active_point->value() != _plot->active_point())
		_active_point->setValue (_plot->active_point());
}


void
EG::update_point_knobs()
{
	unsigned int p = _active_point->value();

	_knob_point_value->param()->set (_envelope_template.points()[p].value * Params::EG::PointValueDenominator);
	_knob_segment_duration->param()->set (p > 0 ? (1.0f * _envelope_template.points()[p-1].samples / ARTIFICIAL_SAMPLE_RATE * Params::EG::SegmentDurationDenominator) : 0);

	_knob_point_value->read();
	_knob_segment_duration->read();
	_knob_segment_duration->setEnabled (p > 0);

	update_params();
}


void
EG::sweep()
{
	for (std::list<Voice*>::iterator v = _dropped_voices.begin(); v != _dropped_voices.end(); ++v)
	{
		EGs::iterator a = _egs.find (*v);
		if (a != _egs.end())
		{
			delete a->second;
			_egs.erase (a);
		}
	}
	_dropped_voices.clear();
}

} // namespace MikuruPrivate

