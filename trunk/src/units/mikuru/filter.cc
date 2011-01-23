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

// Qt:
#include <QtGui/QWhatsThis>
#include <QtGui/QToolTip>

// Haruhi:
#include <haruhi/utility/numeric.h>

// Local:
#include "mikuru.h"
#include "part.h"
#include "filter.h"
#include "event_dispatcher.h"
#include "params.h"


namespace MikuruPrivate {

Filter::Filter (FilterID filter_id, Haruhi::PortGroup* port_group, QString const& port_prefix, QString const& label, Part* part, Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_loading_params (false),
	_part (part),
	_impulse_response (RBJImpulseResponse::LowPass, 0, 0, 0, 1.0),
	_polyphonic_control (part != 0),
	_filter_id (filter_id)
{
	Params::Filter p = _params;

	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	_filter_label = new StyledCheckBoxLabel (label, this);
	_filter_label->checkbox()->setChecked (p.enabled);
	QObject::connect (_filter_label->checkbox(), SIGNAL (clicked()), this, SLOT (update_params()));
	QObject::connect (_filter_label->checkbox(), SIGNAL (clicked()), this, SLOT (update_widgets()));

	_panel = new QWidget (this);
	_panel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	// Plot:
	QFrame* plot_frame = new QFrame (_panel);
	plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_response_plot = new Haruhi::FrequencyResponsePlot (plot_frame);
	QVBoxLayout* plot_frame_layout = new QVBoxLayout (plot_frame, 0, Config::Spacing);
	plot_frame_layout->addWidget (_response_plot);

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	_port_frequency = new Haruhi::EventPort (_mikuru, (port_prefix + " - Frequency").toStdString(), Haruhi::Port::Input, port_group, _polyphonic_control ? Haruhi::Port::Polyphonic : 0);
	_port_resonance = new Haruhi::EventPort (_mikuru, (port_prefix + " - Resonance").toStdString(), Haruhi::Port::Input, port_group, _polyphonic_control ? Haruhi::Port::Polyphonic : 0);
	_port_gain = new Haruhi::EventPort (_mikuru, (port_prefix + " - Gain").toStdString(), Haruhi::Port::Input, port_group, _polyphonic_control ? Haruhi::Port::Polyphonic : 0);
	_port_attenuation = new Haruhi::EventPort (_mikuru, (port_prefix + " - Attenuate").toStdString(), Haruhi::Port::Input, port_group, _polyphonic_control ? Haruhi::Port::Polyphonic : 0);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();

	_knob_frequency = new Haruhi::Knob (_panel, _port_frequency, &_params.frequency, "Freq.", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Filter::Frequency, 2400), 2);
	_knob_resonance = new Haruhi::Knob (_panel, _port_resonance, &_params.resonance, "Q", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Filter::Resonance, 100), 2);
	_knob_gain = new Haruhi::Knob (_panel, _port_gain, &_params.gain, "Gain", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Filter::Gain, 100), 1);
	_knob_attenuation = new Haruhi::Knob (_panel, _port_attenuation, &_params.attenuation, "Attenuate",
										  -std::numeric_limits<float>::infinity(), 0.0f,
										  (Params::Filter::AttenuationMax - Params::Filter::AttenuationMin) / 500, 2);

	_knob_frequency->controller_proxy().config().curve = 1.0;
	_knob_frequency->controller_proxy().config().user_limit_min = 0.04 * Params::Filter::FrequencyDenominator;
	_knob_frequency->controller_proxy().config().user_limit_max = 22.0 * Params::Filter::FrequencyDenominator;
	_knob_frequency->controller_proxy().apply_config();

	_knob_attenuation->controller_proxy().config().curve = 1.0;
	_knob_attenuation->controller_proxy().apply_config();
	_knob_attenuation->set_volume_scale (true, _params.passes);

	QObject::connect (_knob_frequency, SIGNAL (changed (int)), this, SLOT (update_frequency_response()));
	QObject::connect (_knob_resonance, SIGNAL (changed (int)), this, SLOT (update_frequency_response()));
	QObject::connect (_knob_gain, SIGNAL (changed (int)), this, SLOT (update_frequency_response()));
	QObject::connect (_knob_attenuation, SIGNAL (changed (int)), this, SLOT (update_frequency_response()));

	if (_polyphonic_control)
	{
		VoiceManager* vm = _part->voice_manager();
		EventDispatcher::VoiceFilterParamReceiver::FilterID fid = _filter_id == Filter1 ? EventDispatcher::VoiceFilterParamReceiver::Filter1 : EventDispatcher::VoiceFilterParamReceiver::Filter2;

		_evdisp_frequency = new EventDispatcher (_port_frequency, _knob_frequency, new EventDispatcher::VoiceFilterParamReceiver (vm, fid, &Params::Filter::frequency));
		_evdisp_resonance = new EventDispatcher (_port_resonance, _knob_resonance, new EventDispatcher::VoiceFilterParamReceiver (vm, fid, &Params::Filter::resonance));
		_evdisp_gain = new EventDispatcher (_port_gain, _knob_gain, new EventDispatcher::VoiceFilterParamReceiver (vm, fid, &Params::Filter::gain));
		_evdisp_attenuation = new EventDispatcher (_port_attenuation, _knob_attenuation, new EventDispatcher::VoiceFilterParamReceiver (vm, fid, &Params::Filter::attenuation));
	}

	// Widgets/knobs:
	_filter_type = new QComboBox (_panel);
	_filter_type->addItem ("Low pass", RBJImpulseResponse::LowPass);
	_filter_type->addItem ("High pass", RBJImpulseResponse::HighPass);
	_filter_type->addItem ("Band pass", RBJImpulseResponse::BandPass);
	_filter_type->addItem ("Notch", RBJImpulseResponse::Notch);
	_filter_type->addItem ("All pass", RBJImpulseResponse::AllPass);
	_filter_type->addItem ("Peaking", RBJImpulseResponse::Peaking);
	_filter_type->addItem ("Low shelf", RBJImpulseResponse::LowShelf);
	_filter_type->addItem ("High shelf", RBJImpulseResponse::HighShelf);
	_filter_type->setCurrentItem (p.type);
	QObject::connect (_filter_type, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_filter_type, SIGNAL (activated (int)), this, SLOT (update_widgets()));
	QObject::connect (_filter_type, SIGNAL (activated (int)), this, SLOT (update_frequency_response()));

	_passes = new QComboBox (_panel);
	_passes->addItem ("1 pass");
	_passes->addItem ("2 passes");
	_passes->addItem ("3 passes");
	_passes->addItem ("4 passes");
	_passes->addItem ("5 passes");
	_passes->setCurrentItem (p.passes.get() - 1);
	QObject::connect (_passes, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_passes, SIGNAL (activated (int)), this, SLOT (update_widgets()));

	_limiter_enabled = new QCheckBox ("Q limiter", _panel);
	_limiter_enabled->setChecked (_params.limiter_enabled);
	QToolTip::add (_limiter_enabled, "Automatic attenuation limit");
	QObject::connect (_limiter_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QObject::connect (_limiter_enabled, SIGNAL (toggled (bool)), this, SLOT (update_frequency_response()));

	// Layouts:

	QVBoxLayout* layout = new QVBoxLayout (this, 0, Config::Spacing);
	layout->addWidget (_filter_label);
	layout->addWidget (_panel);

	QHBoxLayout* panel_layout = new QHBoxLayout (_panel, 0, Config::Spacing);

	QHBoxLayout* hor1_layout = new QHBoxLayout (panel_layout, Config::Spacing);
	hor1_layout->addWidget (plot_frame);

	QVBoxLayout* ver1_layout = new QVBoxLayout (hor1_layout, Config::Spacing);

	QHBoxLayout* hor2_layout = new QHBoxLayout (ver1_layout, Config::Spacing);
	hor2_layout->addWidget (_filter_type);
	hor2_layout->addWidget (_passes);
	hor2_layout->addWidget (_limiter_enabled);

	QHBoxLayout* hor3_layout = new QHBoxLayout (ver1_layout, Config::Spacing);

	QHBoxLayout* params_layout = new QHBoxLayout (hor3_layout, Config::Spacing);
	params_layout->addWidget (_knob_frequency);
	params_layout->addWidget (_knob_resonance);
	params_layout->addWidget (_knob_gain);
	params_layout->addWidget (_knob_attenuation);

	_response_plot->assign_impulse_response (&_impulse_response);

	update_widgets();
	update_frequency_response();
}


Filter::~Filter()
{
	// Deassign filter before deletion by Qt:
	_response_plot->assign_impulse_response (0);

	// Delete knobs before ControllerProxies:
	delete _knob_frequency;
	delete _knob_resonance;
	delete _knob_gain;
	delete _knob_attenuation;

	if (_polyphonic_control)
	{
		delete _evdisp_frequency;
		delete _evdisp_resonance;
		delete _evdisp_gain;
		delete _evdisp_attenuation;
	}

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	delete _port_frequency;
	delete _port_resonance;
	delete _port_gain;
	delete _port_attenuation;
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
Filter::process_events()
{
	if (_polyphonic_control)
	{
		_evdisp_frequency->load_events();
		_evdisp_resonance->load_events();
		_evdisp_gain->load_events();
		_evdisp_attenuation->load_events();
	}
	else
	{
		_knob_frequency->controller_proxy().process_events();
		_knob_resonance->controller_proxy().process_events();
		_knob_gain->controller_proxy().process_events();
		_knob_attenuation->controller_proxy().process_events();
	}
}


void
Filter::unit_bay_assigned()
{
	_knob_frequency->set_unit_bay (_mikuru->unit_bay());
	_knob_resonance->set_unit_bay (_mikuru->unit_bay());
	_knob_gain->set_unit_bay (_mikuru->unit_bay());
	_knob_attenuation->set_unit_bay (_mikuru->unit_bay());
}


void
Filter::load_params()
{
	Params::Filter p (_params);
	_loading_params = true;

	_filter_label->checkbox()->setChecked (p.enabled);
	_filter_type->setCurrentItem (p.type);
	_passes->setCurrentItem (p.passes.get() - 1);
	_limiter_enabled->setChecked (p.limiter_enabled);

	_loading_params = false;
	update_widgets();
}


void
Filter::load_params (Params::Filter& params)
{
	_params = params;
	load_params();
}


void
Filter::update_params()
{
	if (_loading_params)
		return;

	Params::Filter p;
	p.enabled = _filter_label->checkbox()->isChecked();
	p.type = _filter_type->currentItem();
	p.passes = _passes->currentItem() + 1;
	p.limiter_enabled = _limiter_enabled->isChecked();
	_params.set_non_controller_params (p);

	// Knob params are updated automatically using #assign_parameter.

	params_updated();
}


void
Filter::update_widgets()
{
	int ft = _filter_type->currentItem();
	_knob_gain->setEnabled (ft == RBJImpulseResponse::Peaking || ft == RBJImpulseResponse::LowShelf || ft == RBJImpulseResponse::HighShelf);
	_panel->setEnabled (_params.enabled.get());
	_limiter_enabled->setEnabled (ft == RBJImpulseResponse::LowPass || ft == RBJImpulseResponse::HighPass || ft == RBJImpulseResponse::BandPass ||
								  ft == RBJImpulseResponse::LowShelf || ft == RBJImpulseResponse::HighShelf);
	// Plot and attenuation should reflect Number of filter passes:
	_response_plot->set_num_passes (_params.passes);
	_response_plot->replot();
	_knob_attenuation->set_volume_scale (true, _params.passes);
	_knob_attenuation->update();
}


void
Filter::update_frequency_response()
{
	params_updated();

	_impulse_response.set_type (static_cast<RBJImpulseResponse::Type> (_params.type.get()));
	_impulse_response.set_frequency (0.5f * _params.frequency.get() / Params::Filter::FrequencyMax);
	_impulse_response.set_resonance (_params.resonance.to_f());
	_impulse_response.set_gain (_params.gain.to_f());
	_impulse_response.set_attenuation (_params.attenuation.to_f());
	_impulse_response.set_limiter (_params.limiter_enabled.get());
	_response_plot->replot();
}

} // namespace MikuruPrivate

