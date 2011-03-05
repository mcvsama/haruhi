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
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/widgets/envelope_plot.h>

#include "../mikuru.h"
#include "../envelopes.h"
#include "../voice.h"
#include "../voice_manager.h"
#include "eg.h"


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

	_envelope_template_mutex.lock();
	_envelope_template.points().push_back (DSP::Envelope::Point (0.5, 0.5 * ARTIFICIAL_SAMPLE_RATE));
	_envelope_template.points().push_back (DSP::Envelope::Point (0.5, 0.5 * ARTIFICIAL_SAMPLE_RATE));
	_envelope_template.points().push_back (DSP::Envelope::Point (0.5, 0));
	_envelope_template_mutex.unlock();

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

	create_knobs (this);

	_enabled = new QCheckBox ("Enabled", this);
	_enabled->setChecked (p.enabled);
	QObject::connect (_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	QFrame* plot_frame = new QFrame (this);
	plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_plot = new Haruhi::EnvelopePlot (plot_frame);
	_plot->set_editable (true, 1.0f * Params::EG::SegmentDurationMax / Params::EG::SegmentDurationDenominator);
	_plot->set_sample_rate (ARTIFICIAL_SAMPLE_RATE);
	_plot->assign_envelope (&_envelope_template);
	QObject::connect (_plot, SIGNAL (envelope_updated()), this, SLOT (changed_envelope()));
	QObject::connect (_plot, SIGNAL (active_point_changed()), this, SLOT (changed_envelope()));

	QGroupBox* grid1 = new QGroupBox (this);
	QGridLayout* grid1_layout = new QGridLayout (grid1);
	grid1->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Maximum);
	grid1_layout->setMargin (3 * Config::Margin);

	grid1_layout->addWidget (new QLabel ("Active point:", grid1), 0, 0);
	_active_point = new QSpinBox (grid1);
	_active_point->setMinimum (0);
	grid1_layout->addWidget (_active_point, 0, 1);
	QObject::connect (_active_point, SIGNAL (valueChanged (int)), this, SLOT (changed_active_point()));

	grid1_layout->addWidget (new QLabel ("Sustain point:", grid1), 1, 0);
	_sustain_point = new QSpinBox (grid1);
	_sustain_point->setMinimum (1);
	grid1_layout->addWidget (_sustain_point, 1, 1);
	QObject::connect (_sustain_point, SIGNAL (valueChanged (int)), this, SLOT (update_widgets()));
	QObject::connect (_sustain_point, SIGNAL (valueChanged (int)), this, SLOT (update_params()));
	QObject::connect (_sustain_point, SIGNAL (valueChanged (int)), this, SLOT (update_plot()));

	_add_point_after_active = new QPushButton (Resources::Icons16::add(), "Add after", this); // TODO Icon (<)
	QObject::connect (_add_point_after_active, SIGNAL (clicked()), this, SLOT (add_point_after_active()));

	_add_point_before_active = new QPushButton (Resources::Icons16::remove(), "Add before", this); // TODO Icon (>)
	QObject::connect (_add_point_before_active, SIGNAL (clicked()), this, SLOT (add_point_before_active()));

	_remove_active_point = new QPushButton (Resources::Icons16::remove(), "Remove active", this);
	QObject::connect (_remove_active_point, SIGNAL (clicked()), this, SLOT (remove_active_point()));

	// Layouts:

	QVBoxLayout* plot_frame_layout = new QVBoxLayout (plot_frame);
	plot_frame_layout->setMargin (0);
	plot_frame_layout->setSpacing (Config::Spacing);
	plot_frame_layout->addWidget (_plot);

	QHBoxLayout* h3 = new QHBoxLayout();
	h3->setSpacing (Config::Spacing);
	h3->addWidget (_remove_active_point);
	h3->addWidget (_add_point_before_active);
	h3->addWidget (_add_point_after_active);
	h3->addWidget (grid1);

	QHBoxLayout* h2 = new QHBoxLayout();
	h2->setSpacing (Config::Spacing);
	h2->addWidget (_knob_point_value);
	h2->addWidget (_knob_segment_duration);

	QVBoxLayout* v1 = new QVBoxLayout();
	v1->setSpacing (Config::Spacing);
	v1->addWidget (_enabled);
	v1->addLayout (h2);

	QHBoxLayout* h1 = new QHBoxLayout();
	h1->setSpacing (Config::Spacing);
	h1->addWidget (plot_frame);
	h1->addLayout (v1);

	QVBoxLayout* v0 = new QVBoxLayout (this);
	v0->setMargin (Config::Margin);
	v0->setSpacing (Config::Spacing);
	v0->addLayout (h1);
	v0->addLayout (h3);
	v0->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}


void
EG::voice_created (VoiceManager* voice_manager, Voice* voice)
{
	if (!_params.enabled.get())
		return;

	unsigned int sample_rate = _mikuru->graph()->sample_rate();
	_envelope_template_mutex.lock();
	DSP::Envelope* env = _egs[voice] = new DSP::Envelope (_envelope_template);
	_envelope_template_mutex.unlock();
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


DSP::Envelope
EG::envelope_template()
{
	DSP::Envelope e;
	_envelope_template_mutex.lock();
	e = _envelope_template;
	_envelope_template_mutex.unlock();
	return e;
}


void
EG::load_params()
{
	// Copy params:
	Params::EG p (_params);
	_loading_params = true;

	_enabled->setChecked (p.enabled);
	_sustain_point->setValue (p.sustain_point + 1);
	// Load points:
	_envelope_template_mutex.lock();
	_envelope_template.points().clear();
	for (size_t i = 0; i < p.segments + 1; ++i)
	{
		_envelope_template.points().push_back (
			DSP::Envelope::Point (1.0 * p.values[i] / Params::EG::PointValueDenominator, p.durations[i])
		);
	}
	_envelope_template_mutex.unlock();

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
	_params.sustain_point.set (_sustain_point->value() - 1);

	// Update points:
	_envelope_template_mutex.lock();
	DSP::Envelope::Points& points = _envelope_template.points();
	size_t s = _params.segments.get() + 1;
	for (size_t i = 0; i < s; ++i)
	{
		_params.values[i].set (points[i].value * Params::EG::PointValueDenominator);
		_params.durations[i].set (points[i].samples);
	}
	_envelope_template_mutex.unlock();

	// Knob params are updated automatically using #assign_parameter.

	_params.sanitize();
}


void
EG::update_plot()
{
	_envelope_template_mutex.lock();

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

	_envelope_template_mutex.unlock();
	_plot->post_plot_shape();
}


void
EG::update_widgets()
{
	if (_updating_widgets)
		return;
	_updating_widgets = true;

	unsigned int const segments = _params.segments.get();

	_sustain_point->setMaximum (segments + 1);
	_active_point->setMaximum (segments);

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
	_envelope_template_mutex.lock();
	_envelope_template.points()[p].value = _point_value.to_f();
	_envelope_template_mutex.unlock();

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
	{
		_envelope_template_mutex.lock();
		_envelope_template.points()[p-1].samples = _segment_duration.to_f() * ARTIFICIAL_SAMPLE_RATE;
		_envelope_template_mutex.unlock();
	}

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
EG::add_point_before_active()
{
	_envelope_template_mutex.lock();

	DSP::Envelope::Points& points = _envelope_template.points();
	DSP::Envelope::Points::iterator p = points.begin();
	unsigned int sr = _mikuru->graph()->sample_rate();
	unsigned int pi = _active_point->value();
	float val = 0.5;

	// Avg of adjacent points.
	// Assuming that points.size() is always >= 2:
	if (pi >= 1)
		val = 0.5 * (points[pi - 1].value + points[pi].value);
	else
		val = points[0].value;

	points.insert (p + pi, DSP::Envelope::Point (val, 0.5 * sr)); // 0.1s default
	_params.segments.set (_params.segments.get() + 1);

	_envelope_template_mutex.unlock();
	update_widgets();
	update_params();
	update_plot();
}


void
EG::add_point_after_active()
{
	// TODO

	update_widgets();
	update_params();
	update_plot();
}


void
EG::remove_active_point()
{
	// TODO

	update_widgets();
	update_params();
	update_plot();
}


void
EG::update_point_knobs()
{
	unsigned int p = _active_point->value();

	_envelope_template_mutex.lock();
	_knob_point_value->param()->set (_envelope_template.points()[p].value * Params::EG::PointValueDenominator);
	_knob_segment_duration->param()->set (p > 0 ? (1.0f * _envelope_template.points()[p-1].samples / ARTIFICIAL_SAMPLE_RATE * Params::EG::SegmentDurationDenominator) : 0);
	_envelope_template_mutex.unlock();

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

