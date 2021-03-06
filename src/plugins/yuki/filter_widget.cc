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
#include <functional>

// Qt:
#include <QLayout>
#include <QGroupBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/widgets/styled_background.h>

// Local:
#include "filter_widget.h"
#include "filter_ir.h"
#include "part.h"
#include "part_manager.h"


namespace Yuki {

using namespace Haruhi::ScreenLiterals;


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
	_knob_frequency		= std::make_unique<Haruhi::Knob> (this, proxies->filter_frequency[filter_no].get(), "Freq.");
	_knob_resonance		= std::make_unique<Haruhi::Knob> (this, proxies->filter_resonance[filter_no].get(), "Q");
	_knob_gain			= std::make_unique<Haruhi::Knob> (this, proxies->filter_gain[filter_no].get(), "Gain");
	_knob_attenuation	= std::make_unique<Haruhi::Knob> (this, proxies->filter_attenuation[filter_no].get(), "Attenuate");

	_knob_attenuation->set_volume_scale (true, _params->stages);

	// Set unit bay on all knobs:

	for (auto* k: { _knob_frequency.get(), _knob_resonance.get(), _knob_gain.get(), _knob_attenuation.get() })
		k->set_unit_bay (_part->part_manager()->plugin()->unit_bay());

	// Top widget, can be disabled with all child widgets:
	_panel = std::make_unique<QWidget> (this);
	_panel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	// Enabled checkbox:
	_enabled_widget = std::make_unique<QCheckBox> (QString ("Filter %1").arg (filter_no + 1), this);
	_enabled_widget->setChecked (_params->enabled);
	QObject::connect (_enabled_widget.get(), SIGNAL (clicked()), this, SLOT (widgets_to_params()));
	QObject::connect (_enabled_widget.get(), SIGNAL (clicked()), this, SLOT (update_widgets()));

	// Plot:
	auto plot_frame = new QFrame (this);
	plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	plot_frame->setMinimumHeight (8_em);
	_response_plot = std::make_unique<Haruhi::FrequencyResponsePlot> (plot_frame);

	auto plot_frame_layout = new QVBoxLayout (plot_frame);
	plot_frame_layout->setMargin (0);
	plot_frame_layout->setSpacing (Config::spacing());
	plot_frame_layout->addWidget (_response_plot.get());

	// Filter type combo box:
	_filter_type = std::make_unique<QComboBox> (this);
	_filter_type->addItem (Resources::Icons16::filter_lpf(), "Low pass", FilterImpulseResponse::LowPass);
	_filter_type->addItem (Resources::Icons16::filter_hpf(), "High pass", FilterImpulseResponse::HighPass);
	_filter_type->addItem (Resources::Icons16::filter_bpf(), "Band pass", FilterImpulseResponse::BandPass);
	_filter_type->addItem (Resources::Icons16::filter_notch(), "Notch", FilterImpulseResponse::Notch);
	_filter_type->addItem (Resources::Icons16::filter_allpass(), "All pass", FilterImpulseResponse::AllPass);
	_filter_type->addItem (Resources::Icons16::filter_peaking(), "Peaking", FilterImpulseResponse::Peaking);
	_filter_type->addItem (Resources::Icons16::filter_lowshelf(), "Low shelf", FilterImpulseResponse::LowShelf);
	_filter_type->addItem (Resources::Icons16::filter_highshelf(), "High shelf", FilterImpulseResponse::HighShelf);
	_filter_type->setIconSize (Resources::Icons16::haruhi().size());
	_filter_type->setCurrentIndex (_params->type);
	QObject::connect (_filter_type.get(), SIGNAL (activated (int)), this, SLOT (widgets_to_params()));
	QObject::connect (_filter_type.get(), SIGNAL (activated (int)), this, SLOT (update_widgets()));
	QObject::connect (_filter_type.get(), SIGNAL (activated (int)), this, SLOT (update_impulse_response()));

	// Stages:
	_stages = std::make_unique<QSpinBox> (this);
	_stages->setSuffix (" stages");
	_stages->setRange (1, 5);
	_stages->setValue (_params->stages.get());
	QObject::connect (_stages.get(), SIGNAL (valueChanged (int)), this, SLOT (widgets_to_params()));
	QObject::connect (_stages.get(), SIGNAL (valueChanged (int)), this, SLOT (update_widgets()));
	QObject::connect (_stages.get(), SIGNAL (valueChanged (int)), this, SLOT (update_impulse_response()));

	// Limiter:
	_limiter_enabled = std::make_unique<QCheckBox> ("Limit", this);
	_limiter_enabled->setChecked (_params->limiter_enabled);
	_limiter_enabled->setToolTip ("Automatic attenuation limit");
	QObject::connect (_limiter_enabled.get(), SIGNAL (toggled (bool)), this, SLOT (widgets_to_params()));
	QObject::connect (_limiter_enabled.get(), SIGNAL (toggled (bool)), this, SLOT (update_impulse_response()));

	// Layouts:
	auto hor2_layout = new QHBoxLayout();
	hor2_layout->setSpacing (Config::spacing());
	hor2_layout->addWidget (_filter_type.get());
	hor2_layout->addWidget (_stages.get());
	hor2_layout->addWidget (_limiter_enabled.get());

	auto params_layout = new QHBoxLayout();
	params_layout->setSpacing (Config::spacing());
	params_layout->addWidget (_knob_frequency.get());
	params_layout->addWidget (_knob_resonance.get());
	params_layout->addWidget (_knob_gain.get());
	params_layout->addWidget (_knob_attenuation.get());

	auto hor3_layout = new QHBoxLayout();
	hor3_layout->setSpacing (Config::spacing());
	hor3_layout->addLayout (params_layout);

	auto ver1_layout = new QVBoxLayout (_panel.get());
	ver1_layout->setMargin (0);
	ver1_layout->setSpacing (Config::spacing());
	ver1_layout->addWidget (plot_frame);
	ver1_layout->addLayout (hor2_layout);
	ver1_layout->addLayout (hor3_layout);

	auto layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addWidget (new Haruhi::StyledBackground (_enabled_widget.get(), this));
	layout->addWidget (_panel.get());

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
	// Deassign filter before deletion:
	_response_plot->assign_impulse_response (nullptr);
}


void
FilterWidget::widgets_to_params()
{
	if (_stop_widgets_to_params)
		return;
	_stop_params_to_widgets = true;

	_params->enabled = _enabled_widget->isChecked();
	_params->type = _filter_type->currentIndex();
	_params->stages = _stages->value();
	_params->limiter_enabled = _limiter_enabled->isChecked();

	_stop_params_to_widgets = false;
}


void
FilterWidget::update_widgets()
{
	int ft = _filter_type->currentIndex();
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
	_filter_type->setCurrentIndex (_params->type);
	_stages->setValue (_params->stages);
	_limiter_enabled->setChecked (_params->limiter_enabled);

	_stop_widgets_to_params = false;

	update_widgets();
}


void
FilterWidget::post_params_to_widgets()
{
	update_impulse_response();
	Haruhi::Services::call_out (std::bind (&FilterWidget::params_to_widgets, this));
}

} // namespace Yuki

