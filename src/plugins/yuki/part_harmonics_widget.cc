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
#include <iterator>

// Qt:
#include <QLayout>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/widgets/styled_background.h>
#include <haruhi/widgets/plot_frame.h>
#include <haruhi/widgets/wave_plot.h>

// Local:
#include "part_harmonics_widget.h"
#include "part_widget.h"
#include "part.h"


namespace Yuki {

namespace DSP = Haruhi::DSP;
using namespace Haruhi::ScreenLiterals;


Slider::Slider (int min_value, int max_value, int page_step, int value, Qt::Orientation orientation, QWidget* parent):
	QSlider (orientation, parent)
{
	setMinimum (min_value);
	setMaximum (max_value);
	setPageStep (page_step);
	setValue (value);
}


void
Slider::reset()
{
	setValue (0);
}


PartHarmonicsWidget::PartHarmonicsWidget (QWidget* parent, PartWidget* part_widget, Part* part):
	QWidget (parent),
	_part_widget (part_widget),
	_part (part)
{
	// Reset all harmonics:
	auto harmonics_reset_button = new QPushButton (Resources::Icons16::clear_list(), "Reset all harmonics and phases to zero", this);
	harmonics_reset_button->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (harmonics_reset_button, SIGNAL (clicked()), this, SLOT (reset_all_harmonics()));

	int const kHarmonicsButtonWidth = 1.6_em;
	int const kHarmonicsButtonHeight = 1.6_em;

	// Harmonic sliders:
	auto harmonics_widget = new QWidget (this);
	harmonics_widget->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	auto harmonics_grid = new QWidget (harmonics_widget);
	auto harmonics_layout = new QGridLayout (harmonics_grid);
	harmonics_layout->setSpacing (0);
	for (Sliders::size_type i = 0; i < Params::Part::HarmonicsNumber; ++i)
	{
		harmonics_layout->setColumnMinimumWidth (i, 0);

		int def = i == 0 ? Params::Part::HarmonicMax : Params::Part::HarmonicDefault;
		auto slider = new Slider (Params::Part::HarmonicMin, Params::Part::HarmonicMax, Params::Part::HarmonicMax / 20, def, Qt::Vertical, harmonics_grid);
		slider->setTracking (true);
		QObject::connect (slider, SIGNAL (valueChanged (int)), _part_widget, SLOT (widgets_to_wave_params()));
		QObject::connect (slider, SIGNAL (valueChanged (int)), _part_widget, SLOT (update_widgets()));
		harmonics_layout->addWidget (slider, 0, i);

		auto label = new QLabel (QString ("%1").arg (i + 1), harmonics_grid);
		label->setAlignment (Qt::AlignCenter);
		label->setFixedWidth (kHarmonicsButtonWidth);
		label->setTextFormat (Qt::PlainText);
		harmonics_layout->addWidget (label, 1, i);

		auto reset = new QPushButton ("∅", harmonics_grid);
		reset->setFixedWidth (kHarmonicsButtonWidth);
		reset->setFixedHeight (kHarmonicsButtonHeight);
		reset->setToolTip ("Reset to zero");
		QObject::connect (reset, SIGNAL (clicked()), slider, SLOT (reset()));
		harmonics_layout->addWidget (reset, 2, i);

		_harmonics_sliders.push_back (slider);
		_harmonics_resets.push_back (reset);
	}
	auto harmonics_tab_layout = new QHBoxLayout (harmonics_widget);
	harmonics_tab_layout->setMargin (0);
	harmonics_tab_layout->setSpacing (0);
	harmonics_tab_layout->addWidget (harmonics_grid);

	// Phase sliders:
	auto harmonic_phases_widget = new QWidget (this);
	harmonic_phases_widget->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	auto phases_grid = new QWidget (harmonic_phases_widget);
	auto phases_layout = new QGridLayout (phases_grid);
	phases_layout->setSpacing (0);
	for (Sliders::size_type i = 0; i < Params::Part::HarmonicsNumber; ++i)
	{
		harmonics_layout->setColumnMinimumWidth (i, 0);

		auto slider = new Slider (Params::Part::PhaseMin, Params::Part::PhaseMax, Params::Part::PhaseMax / 20, Params::Part::PhaseDefault, Qt::Vertical, phases_grid);
		slider->setTracking (true);
		QObject::connect (slider, SIGNAL (valueChanged (int)), _part_widget, SLOT (widgets_to_wave_params()));
		QObject::connect (slider, SIGNAL (valueChanged (int)), _part_widget, SLOT (update_widgets()));
		phases_layout->addWidget (slider, 0, i);

		auto label = new QLabel (QString ("%1").arg (i + 1), phases_grid);
		label->setAlignment (Qt::AlignCenter);
		label->setFixedWidth (kHarmonicsButtonWidth);
		label->setTextFormat (Qt::PlainText);
		phases_layout->addWidget (label, 1, i);

		auto reset = new QPushButton ("∅", phases_grid);
		reset->setFixedWidth (kHarmonicsButtonWidth);
		reset->setFixedHeight (kHarmonicsButtonHeight);
		reset->setToolTip ("Reset to zero");
		phases_layout->addWidget (reset, 2, i);

		QObject::connect (reset, SIGNAL (clicked()), slider, SLOT (reset()));

		_harmonic_phases_sliders.push_back (slider);
		_harmonic_phases_resets.push_back (reset);
	}
	auto phases_tab_layout = new QHBoxLayout (harmonic_phases_widget);
	phases_tab_layout->setMargin (0);
	phases_tab_layout->setSpacing (0);
	phases_tab_layout->addWidget (phases_grid);

	auto waves_label = new QLabel ("Base & output waves", this);
	// Force normal text color. For some reason Qt uses white color on light-gray background.
	waves_label->setForegroundRole (QPalette::Text);

	auto waves_label_background = new Haruhi::StyledBackground (waves_label, this, 0.12_em);
	waves_label_background->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	waves_label_background->setFixedWidth (12_em); // Same width as wave plots

	_base_wave_plot = std::make_unique<Haruhi::WavePlot> (this);
	_base_wave_plot->set_phase_marker_enabled (true);
	_base_wave_plot->set_closed_ring (true);
	_base_wave_plot->set_filled_wave (true);

	auto base_plot_frame = new Haruhi::PlotFrame (this);
	base_plot_frame->set_widget (_base_wave_plot.get());
	base_plot_frame->setToolTip ("Base wave");
	base_plot_frame->setFixedSize (12_em, 8_em);

	_final_wave_plot = std::make_unique<Haruhi::WavePlot> (this);
	_final_wave_plot->set_phase_marker_enabled (true);
	_final_wave_plot->set_closed_ring (true);
	_final_wave_plot->set_filled_wave (true);

	auto final_plot_frame = new Haruhi::PlotFrame (this);
	final_plot_frame->set_widget (_final_wave_plot.get());
	final_plot_frame->setToolTip ("Output wave (with harmonics, modulation, auto-scaled, etc.)");
	final_plot_frame->setFixedSize (12_em, 8_em);

	// Layouts:

	auto harmonics_label = new QLabel ("Harmonics", this);
	// Force normal text color. For some reason Qt uses white color on light-gray background.
	harmonics_label->setForegroundRole (QPalette::Text);
	auto harmonic_phases_label = new QLabel ("Phases", this);
	// Force normal text color. For some reason Qt uses white color on light-gray background.
	harmonic_phases_label->setForegroundRole (QPalette::Text);

	auto harmonics_panel_buttons_layout = new QHBoxLayout();
	harmonics_panel_buttons_layout->setMargin (0);
	harmonics_panel_buttons_layout->setSpacing (Config::spacing());
	harmonics_panel_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	harmonics_panel_buttons_layout->addWidget (harmonics_reset_button);

	auto harmonics_panel_layout = new QVBoxLayout();
	harmonics_panel_layout->setMargin (0);
	harmonics_panel_layout->setSpacing (Config::spacing());
	harmonics_panel_layout->addWidget (new Haruhi::StyledBackground (harmonics_label, this, 0.12_em));
	harmonics_panel_layout->addWidget (harmonics_widget);
	harmonics_panel_layout->addWidget (new Haruhi::StyledBackground (harmonic_phases_label, this, 0.12_em));
	harmonics_panel_layout->addWidget (harmonic_phases_widget);
	harmonics_panel_layout->addLayout (harmonics_panel_buttons_layout);

	auto layout = new QGridLayout (this);
	layout->setMargin (Config::margin());
	layout->setSpacing (Config::spacing());
	layout->addWidget (waves_label_background, 0, 0);
	layout->addWidget (base_plot_frame, 1, 0);
	layout->addWidget (final_plot_frame, 2, 0);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 3, 0);
	layout->addLayout (harmonics_panel_layout, 0, 1, 4, 1);

	// Save standard button colors:
	_std_button_bg = _harmonics_resets[0]->palette().color (QPalette::Button);
	_std_button_fg = _harmonics_resets[0]->palette().color (QPalette::ButtonText);
}


void
PartHarmonicsWidget::widgets_to_wave_params()
{
	auto pp = _part->part_params();

	// Update params that are not controller by knobs:
	for (std::size_t i = 0; i < std::size (pp->harmonics); ++i)
		pp->harmonics[i].set (_harmonics_sliders[i]->value());
	for (std::size_t i = 0; i < std::size (pp->harmonic_phases); ++i)
		pp->harmonic_phases[i].set (_harmonic_phases_sliders[i]->value());
}


void
PartHarmonicsWidget::update_widgets()
{
	for (Sliders::size_type i = 0; i < Params::Part::HarmonicsNumber; ++i)
	{
		set_button_highlighted (_harmonics_resets[i], _harmonics_sliders[i]->value() != 0);
		set_button_highlighted (_harmonic_phases_resets[i], _harmonic_phases_sliders[i]->value() != 0);
	}
}


void
PartHarmonicsWidget::params_to_widgets()
{
	auto pp = _part->part_params();

	for (std::size_t i = 0; i < std::size (pp->harmonics); ++i)
		_harmonics_sliders[i]->setValue (pp->harmonics[i]);
	for (std::size_t i = 0; i < std::size (pp->harmonic_phases); ++i)
		_harmonic_phases_sliders[i]->setValue (pp->harmonic_phases[i]);
}


void
PartHarmonicsWidget::update_phase_marker (float pos)
{
	_base_wave_plot->set_phase_marker_position (pos);
	_base_wave_plot->post_plot_shape();
	_final_wave_plot->set_phase_marker_position (pos);
	_final_wave_plot->post_plot_shape();
}


void
PartHarmonicsWidget::update_wave_plots (DSP::Wave* base_wave, DSP::Wave* final_wave)
{
	_base_wave_plot->assign_wave (base_wave, false, true, false);
	_final_wave_plot->assign_wave (final_wave, false, true, false);
}


void
PartHarmonicsWidget::reset_all_harmonics()
{
	for (QSlider* s: _harmonics_sliders)
		static_cast<Slider*> (s)->reset();
	for (QSlider* s: _harmonic_phases_sliders)
		static_cast<Slider*> (s)->reset();
}


void
PartHarmonicsWidget::set_button_highlighted (QPushButton* button, bool highlight)
{
	QPalette p = button->palette();
	p.setColor (QPalette::Button, highlight ? QColor (0x00, 0xff, 0x00) : _std_button_bg);
	p.setColor (QPalette::ButtonText, highlight ? QColor (0x00, 0x00, 0x00) : _std_button_fg);
	button->setPalette (p);
}

} // namespace Yuki

