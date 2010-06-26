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
	_buffer (mikuru->graph()->buffer_size())
{
	_id = (id == 0) ? _mikuru->allocate_id ("envs") : _mikuru->reserve_id ("envs", id);

	_envelope_template.points().push_back (DSP::Envelope::Point (1.0, 0));

	create_ports();
	create_proxies();
	create_widgets();
	update_plot();
}


EG::~EG()
{
	_plot->assign_envelope (0);
	_mikuru->free_id ("envs", _id);

	// Delete knobs before ControllerProxies:
	delete _control_segment_duration;

	delete _proxy_segment_duration;

	_mikuru->graph()->lock();
	delete _port_segment_duration;

	sweep();
	// Delete remaining EGs:
	for (EGs::iterator x = _egs.begin(); x != _egs.end(); ++x)
	{
		x->first->set_tracked (false);
		delete x->second;
	}
	_mikuru->graph()->unlock();
}


void
EG::create_ports()
{
	_mikuru->graph()->lock();
	_port_group = new Core::PortGroup (_mikuru->graph(), QString ("EG %1").arg (this->id()).toStdString());
	// Inputs:
	_port_segment_duration = new Core::EventPort (_mikuru, "Duration", Core::Port::Input, _port_group);
	// Outputs:
	// TODO
	_mikuru->graph()->unlock();
}


void
EG::create_proxies()
{
	_proxy_segment_duration = new ControllerProxy (_port_segment_duration, new int, 0, HARUHI_MIKURU_MINMAX (Params::EG::SegmentDuration), 0); // XXX new int
	_proxy_segment_duration->config()->curve = 1.0;
	_proxy_segment_duration->apply_config();
}


void
EG::create_knobs (QWidget* parent)
{
	_control_segment_duration = new Knob (parent, _proxy_segment_duration, "Duration", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::EG::SegmentDuration, 100), 2);
	_control_segment_duration->set_unit_bay (_mikuru->unit_bay());

	QObject::connect (_control_segment_duration, SIGNAL (changed (int)), this, SLOT (update_plot()));
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
	_plot = new EnvelopePlot (plot_frame);
	_plot->set_editable (true);
	QVBoxLayout* plot_frame_layout = new QVBoxLayout (plot_frame, 0, Config::spacing);
	plot_frame_layout->addWidget (_plot);

	Q3GroupBox* grid1 = new Q3GroupBox (2, Qt::Horizontal, "", this);
	grid1->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	grid1->setInsideMargin (3 * Config::margin);

	_enabled = new QCheckBox ("Enabled", grid1);
	_enabled->setChecked (p.enabled);
	QObject::connect (_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	// TODO
	_sustain_point = new QSpinBox (grid1);
	_total_points = new QSpinBox (grid1);
	_total_points->setMinimum (1);
	_total_points->setMaximum (64);

	QVBoxLayout* v1 = new QVBoxLayout (knobs_panel, 0, Config::spacing);
	QHBoxLayout* h1 = new QHBoxLayout (v1, Config::spacing);
	h1->addWidget (plot_frame);
	QHBoxLayout* h2 = new QHBoxLayout (v1, Config::spacing);
	h2->addWidget (_control_segment_duration);
	v1->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	QHBoxLayout* h3 = new QHBoxLayout (this, Config::margin, Config::spacing);
	h3->addWidget (knobs_panel);
	QVBoxLayout* v2 = new QVBoxLayout (h3, Config::spacing);
	v2->addWidget (grid1);
	v2->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}


void
EG::voice_created (VoiceManager* voice_manager, Voice* voice)
{
	if (!atomic (_params.enabled))
		return;

	// Minimum attack/release time (ms), prevents clicking:
	_egs[voice] = new DSP::Envelope (_envelope_template);
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

	// TODO
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
	_sustain_point->setValue (p.sustain_point);
	_total_points->setValue (p.points);

	_loading_params = false;
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

	atomic (_params.enabled) = _enabled->isChecked();
	atomic (_params.points) = _total_points->value();
	atomic (_params.sustain_point) = _sustain_point->value();

	// Knob params are updated automatically using #assign_parameter.
}


void
EG::update_plot()
{
	// If graph was not connected, sample rate would be 0 which would cause plotting problems.
	_envelope_template.set_sustain_point (_params.sustain_point);
	_plot->set_sample_rate (SR_FOR_TEMPLATE);
	_plot->assign_envelope (&_envelope_template);
	_plot->plot_shape();
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

