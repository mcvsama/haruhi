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
#include <QLayout>

// Lib:
#include <boost/bind.hpp>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/widgets/plot_frame.h>
#include <haruhi/widgets/wave_plot.h>
#include <haruhi/widgets/styled_background.h>
#include <haruhi/dsp/translated_wave.h>
#include <haruhi/dsp/scaled_wave.h>

// Local:
#include "part_widget.h"
#include "params.h"
#include "part_oscillator_widget.h"
#include "part_modulator_widget.h"
#include "part_harmonics_widget.h"
#include "part_manager_widget.h"
#include "part_manager.h"
#include "part.h"
#include "filter_widget.h"


namespace Yuki {

using namespace Haruhi::ScreenLiterals;


PartWidget::PartWidget (PartManagerWidget* part_manager_widget, Part* part):
	QWidget (part_manager_widget),
	_part_manager_widget (part_manager_widget),
	_part (part),
	_stop_widgets_to_params (false),
	_stop_params_to_widgets (false)
{
	// Part enabled:
	_part_enabled = std::make_unique<QCheckBox> ("Enabled", this);
	_part_enabled->setChecked (_part->part_params()->part_enabled);
	QObject::connect (_part_enabled.get(), SIGNAL (toggled (bool)), this, SLOT (widgets_to_oscillator_params()));
	QObject::connect (_part_enabled.get(), SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	// Oscillator tab:
	_oscillator_panel = std::make_unique<PartOscillatorWidget> (this, this, _part);
	_oscillator_panel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	// Modulator tab:
	_modulator_panel = std::make_unique<PartModulatorWidget> (this, this, _part);
	_modulator_panel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	// Harmonics+phases tab:
	_harmonics_panel = std::make_unique<PartHarmonicsWidget> (this, this, _part);
	_harmonics_panel->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

	// Tabs:
	_tabs = std::make_unique<QTabWidget> (this);
	_tabs->setIconSize (Resources::Icons16::haruhi().size() * 1.25);
	_tabs->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_tabs->addTab (_oscillator_panel.get(), Resources::Icons16::oscillator(), "Oscillator");
	_tabs->addTab (_modulator_panel.get(), Resources::Icons16::modulator(), "Modulator");
	_tabs->addTab (_harmonics_panel.get(), Resources::Icons16::harmonics(), "Harmonics");

	auto top_checkboxes = new QWidget (this);

	auto top_checkboxes_layout = new QHBoxLayout (top_checkboxes);
	top_checkboxes_layout->setMargin (0);
	top_checkboxes_layout->setSpacing (2 * Config::spacing());
	top_checkboxes_layout->addWidget (_part_enabled.get());

	auto layout = new QVBoxLayout (this);
	layout->setMargin (Config::margin());
	layout->setSpacing (Config::spacing());
	layout->addWidget (new Haruhi::StyledBackground (top_checkboxes, this));
	layout->addWidget (_tabs.get());
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	widgets_to_wave_params();
	update_widgets();
}


PartWidget::~PartWidget()
{ }


void
PartWidget::widgets_to_wave_params()
{
	if (_stop_widgets_to_params)
		return;
	_stop_params_to_widgets = true;

	_oscillator_panel->widgets_to_wave_params();
	_harmonics_panel->widgets_to_wave_params();

	update_wave_plots();

	_stop_params_to_widgets = false;
}


void
PartWidget::widgets_to_oscillator_params()
{
	if (_stop_widgets_to_params)
		return;
	_stop_params_to_widgets = true;

	_part->part_params()->part_enabled = _part_enabled->isChecked();

	_oscillator_panel->widgets_to_oscillator_params();
	_modulator_panel->widgets_to_oscillator_params();

	_stop_params_to_widgets = false;
}


void
PartWidget::update_widgets()
{
	bool const enabled = _part->part_params()->part_enabled.get();
	bool const wave_enabled = _oscillator_panel->is_wave_enabled();
	bool const immutable = _part->base_wave()->immutable();

	_harmonics_panel->setEnabled (enabled && wave_enabled && immutable);
	_oscillator_panel->setEnabled (enabled);
	_modulator_panel->setEnabled (enabled);

	_oscillator_panel->update_widgets();
	_harmonics_panel->update_widgets();
	_modulator_panel->update_widgets();
}


void
PartWidget::update_phase_marker()
{
	float pos = 0.5f * (1.0f + _part->part_params()->phase.to_f());

	_oscillator_panel->update_phase_marker (pos);
	_harmonics_panel->update_phase_marker (pos);
}


void
PartWidget::params_to_widgets()
{
	if (_stop_params_to_widgets)
		return;
	_stop_widgets_to_params = true;

	_part_enabled->setChecked (_part->part_params()->part_enabled);

	_oscillator_panel->params_to_widgets();
	_harmonics_panel->params_to_widgets();
	_modulator_panel->params_to_widgets();

	_stop_widgets_to_params = false;

	update_widgets();
}


void
PartWidget::update_wave_plots()
{
	Unique<DSP::Wave> previous_final_wave = std::move (_cached_final_wave);

	_cached_final_wave = _part->final_wave();

	DSP::Wave* base_wave = _cached_final_wave.get();
	while (base_wave->inner_wave())
		base_wave = base_wave->inner_wave();

	_oscillator_panel->update_wave_plots (base_wave, _cached_final_wave.get());
	_harmonics_panel->update_wave_plots (base_wave, _cached_final_wave.get());

	// This will also call plot_shape() on plots:
	update_phase_marker();
}


void
PartWidget::post_update_wave_plots()
{
	Haruhi::Services::call_out (boost::bind (&PartWidget::update_wave_plots, this));
}


void
PartWidget::post_params_to_widgets()
{
	Haruhi::Services::call_out (boost::bind (&PartWidget::params_to_widgets, this));
}

} // namespace Yuki

