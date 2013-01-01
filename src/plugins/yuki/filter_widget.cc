/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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
#include <QtGui/QToolTip>
#include <QtGui/QLayout>
#include <QtGui/QGroupBox>

// Lib:
#include <boost/bind.hpp>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "filter_widget.h"
#include "filter_ir.h"
#include "part.h"
#include "part_manager.h"
#include "widgets.h"


namespace Yuki {

FilterWidget::FilterWidget (QWidget* parent, unsigned int filter_no, Params::Filter* params, Part* part):
	QWidget (parent),
	_params (params),
	_part (part),
	_stop_widgets_to_params (false),
	_stop_params_to_widgets (false)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	// Knobs:

	Part::PartControllerProxies* proxies = part->proxies();
	_knob_frequency		= new Haruhi::Knob (this, proxies->filter_frequency[filter_no], "Freq.");
	_knob_resonance		= new Haruhi::Knob (this, proxies->filter_resonance[filter_no], "Q");
	_knob_gain			= new Haruhi::Knob (this, proxies->filter_gain[filter_no], "Gain");
	_knob_attenuation	= new Haruhi::Knob (this, proxies->filter_attenuation[filter_no], "Attenuate");

	_knob_attenuation->set_volume_scale (true, _params->stages);

	// Set unit bay on all knobs:

	for (auto* k: { _knob_frequency, _knob_resonance, _knob_gain, _knob_attenuation })
		k->set_unit_bay (_part->part_manager()->plugin()->unit_bay());

	// Top widget, can be disabled with all child widgets:
	_panel = new QWidget (this);
	_panel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	// Enabled checkbox:
	_enabled_widget = new QCheckBox (QString ("Filter %1").arg (filter_no + 1), this);
	_enabled_widget->setChecked (_params->enabled);
	QObject::connect (_enabled_widget, SIGNAL (clicked()), this, SLOT (widgets_to_params()));
	QObject::connect (_enabled_widget, SIGNAL (clicked()), this, SLOT (update_widgets()));

	// Plot:
	QFrame* plot_frame = new QFrame (this);
	plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	plot_frame->setMinimumHeight (80);
	_response_plot = new Haruhi::FrequencyResponsePlot (plot_frame);
	QVBoxLayout* plot_frame_layout = new QVBoxLayout (plot_frame);
	plot_frame_layout->setMargin (0);
	plot_frame_layout->setSpacing (Config::Spacing);
	plot_frame_layout->addWidget (_response_plot);

	// Filter type combo box:
	_filter_type = new QComboBox (this);
	_filter_type->addItem (Resources::Icons16::filter_lpf(), "Low pass", FilterImpulseResponse::LowPass);
	_filter_type->addItem (Resources::Icons16::filter_hpf(), "High pass", FilterImpulseResponse::HighPass);
	_filter_type->addItem (Resources::Icons16::filter_bpf(), "Band pass", FilterImpulseResponse::BandPass);
	_filter_type->addItem (Resources::Icons16::filter_notch(), "Notch", FilterImpulseResponse::Notch);
	_filter_type->addItem (Resources::Icons16::filter_allpass(), "All pass", FilterImpulseResponse::AllPass);
	_filter_type->addItem (Resources::Icons16::filter_peaking(), "Peaking", FilterImpulseResponse::Peaking);
	_filter_type->addItem (Resources::Icons16::filter_lowshelf(), "Low shelf", FilterImpulseResponse::LowShelf);
	_filter_type->addItem (Resources::Icons16::filter_highshelf(), "High shelf", FilterImpulseResponse::HighShelf);
	_filter_type->setCurrentItem (_params->type);
	QObject::connect (_filter_type, SIGNAL (activated (int)), this, SLOT (widgets_to_params()));
	QObject::connect (_filter_type, SIGNAL (activated (int)), this, SLOT (update_widgets()));
	QObject::connect (_filter_type, SIGNAL (activated (int)), this, SLOT (update_impulse_response()));

	// Stages:
	_stages = new QSpinBox (this);
	_stages->setSuffix (" stages");
	_stages->setRange (1, 5);
	_stages->setValue (_params->stages.get());
	QObject::connect (_stages, SIGNAL (valueChanged (int)), this, SLOT (widgets_to_params()));
	QObject::connect (_stages, SIGNAL (valueChanged (int)), this, SLOT (update_widgets()));
	QObject::connect (_stages, SIGNAL (valueChanged (int)), this, SLOT (update_impulse_response()));

	// Limiter:
	_limiter_enabled = new QCheckBox ("Limit", this);
	_limiter_enabled->setChecked (_params->limiter_enabled);
	QToolTip::add (_limiter_enabled, "Automatic attenuation limit");
	QObject::connect (_limiter_enabled, SIGNAL (toggled (bool)), this, SLOT (widgets_to_params()));
	QObject::connect (_limiter_enabled, SIGNAL (toggled (bool)), this, SLOT (update_impulse_response()));

	// Layouts:
	QHBoxLayout* hor2_layout = new QHBoxLayout();
	hor2_layout->setSpacing (Config::Spacing);
	hor2_layout->addWidget (_filter_type);
	hor2_layout->addWidget (_stages);
	hor2_layout->addWidget (_limiter_enabled);

	QHBoxLayout* params_layout = new QHBoxLayout();
	params_layout->setSpacing (Config::Spacing);
	params_layout->addWidget (_knob_frequency);
	params_layout->addWidget (_knob_resonance);
	params_layout->addWidget (_knob_gain);
	params_layout->addWidget (_knob_attenuation);

	QHBoxLayout* hor3_layout = new QHBoxLayout();
	hor3_layout->setSpacing (Config::Spacing);
	hor3_layout->addLayout (params_layout);

	QVBoxLayout* ver1_layout = new QVBoxLayout (_panel);
	ver1_layout->setMargin (0);
	ver1_layout->setSpacing (Config::Spacing);
	ver1_layout->addWidget (plot_frame);
	ver1_layout->addLayout (hor2_layout);
	ver1_layout->addLayout (hor3_layout);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (new StyledBackground (_enabled_widget, this));
	layout->addWidget (_panel);

	_response_plot->assign_impulse_response (&_impulse_response);

	// Update impulse response and plot when params are changed:
	_params->frequency.on_change.connect (this, &FilterWidget::update_impulse_response);
	_params->resonance.on_change.connect (this, &FilterWidget::update_impulse_response);
	_params->gain.on_change.connect (this, &FilterWidget::update_impulse_response);
	_params->attenuation.on_change.connect (this, &FilterWidget::update_impulse_response);
	// Other params:
	_params->enabled.on_change.connect (this, &FilterWidget::post_params_to_widgets);
	_params->type.on_change.connect (this, &FilterWidget::post_params_to_widgets);
	_params->stages.on_change.connect (this, &FilterWidget::post_params_to_widgets);
	_params->limiter_enabled.on_change.connect (this, &FilterWidget::post_params_to_widgets);

	update_widgets();
	update_impulse_response();
}


FilterWidget::~FilterWidget()
{
	// Deassign filter before deletion by Qt:
	_response_plot->assign_impulse_response (0);

	delete _knob_frequency;
	delete _knob_resonance;
	delete _knob_gain;
	delete _knob_attenuation;
}


void
FilterWidget::widgets_to_params()
{
	if (_stop_widgets_to_params)
		return;
	_stop_params_to_widgets = true;

	_params->enabled = _enabled_widget->isChecked();
	_params->type = _filter_type->currentItem();
	_params->stages = _stages->value();
	_params->limiter_enabled = _limiter_enabled->isChecked();

	_stop_params_to_widgets = false;
}


void
FilterWidget::update_widgets()
{
	int ft = _filter_type->currentItem();
	_panel->setEnabled (_params->enabled.get());
	_knob_gain->setEnabled (ft == FilterImpulseResponse::Peaking || ft == FilterImpulseResponse::LowShelf || ft == FilterImpulseResponse::HighShelf);
	_limiter_enabled->setEnabled (ft == FilterImpulseResponse::LowPass || ft == FilterImpulseResponse::HighPass || ft == FilterImpulseResponse::BandPass ||
								  ft == FilterImpulseResponse::Peaking || ft == FilterImpulseResponse::LowShelf || ft == FilterImpulseResponse::HighShelf);
	// Plot and attenuation should reflect Number of filter stages:
	_response_plot->set_num_stages (_params->stages);
	_response_plot->replot();
	_knob_attenuation->set_volume_scale (true, _params->stages);
	_knob_attenuation->update();
}


void
FilterWidget::update_impulse_response()
{
	_impulse_response.set_type (static_cast<FilterImpulseResponse::Type> (_params->type.get()));
	_impulse_response.set_frequency (0.5f * _params->frequency.get() / Params::Filter::FrequencyMax);
	_impulse_response.set_resonance (_params->resonance.to_f());
	_impulse_response.set_gain (_params->gain.to_f());
	_impulse_response.set_attenuation (_params->attenuation.to_f());
	_impulse_response.set_limiter_enabled (_params->limiter_enabled.get());

	_response_plot->post_replot();
}


void
FilterWidget::params_to_widgets()
{
	if (_stop_params_to_widgets)
		return;
	_stop_widgets_to_params = true;

	_enabled_widget->setChecked (_params->enabled);
	_filter_type->setCurrentItem (_params->type);
	_stages->setValue (_params->stages);
	_limiter_enabled->setChecked (_params->limiter_enabled);

	_stop_widgets_to_params = false;

	update_widgets();
}


void
FilterWidget::post_params_to_widgets()
{
	update_impulse_response();
	Haruhi::Services::call_out (boost::bind (&FilterWidget::params_to_widgets, this));
}

} // namespace Yuki

