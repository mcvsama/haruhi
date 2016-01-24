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
#include <QtGui/QLayout>
#include <QtGui/QGroupBox>

// Lib:
#include <boost/bind.hpp>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/widgets/plot_frame.h>
#include <haruhi/widgets/styled_background.h>
#include <haruhi/dsp/modulated_wave.h>
#include <haruhi/dsp/translated_wave.h>
#include <haruhi/dsp/scaled_wave.h>

// Local:
#include "part_harmonics.h"
#include "part_widget.h"
#include "part.h"


namespace Yuki {

using namespace Haruhi::ScreenLiterals;


PartHarmonicsWidget::PartHarmonicsWidget (QWidget* parent, PartWidget* part_widget, Part* part):
	QWidget (parent),
	_part_widget (part_widget),
	_part (part)
{
	// Reset all harmonics:
	QPushButton* harmonics_reset_button = new QPushButton (Resources::Icons16::clear_list(), "Clear all harmonics and phases", this);
	harmonics_reset_button->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (harmonics_reset_button, SIGNAL (clicked()), this, SLOT (reset_all_harmonics()));

	int const kHarmonicsButtonWidth = 4_screen_mm;
	int const kHarmonicsButtonHeight = 3_screen_mm;

	// Harmonic sliders:
	QWidget* harmonics_widget = new QWidget (this);
	harmonics_widget->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QWidget* harmonics_grid = new QWidget (harmonics_widget);
	QGridLayout* harmonics_layout = new QGridLayout (harmonics_grid);
	harmonics_layout->setSpacing (0);
	for (Sliders::size_type i = 0; i < Params::Part::HarmonicsNumber; ++i)
	{
		harmonics_layout->setColumnMinimumWidth (i, 0);

		int def = i == 0 ? Params::Part::HarmonicMax : Params::Part::HarmonicDefault;
		Slider* slider = new Slider (Params::Part::HarmonicMin, Params::Part::HarmonicMax, Params::Part::HarmonicMax / 20, def, Qt::Vertical, harmonics_grid);
		slider->setTracking (true);
		QObject::connect (slider, SIGNAL (valueChanged (int)), _part_widget, SLOT (widgets_to_wave_params()));
		QObject::connect (slider, SIGNAL (valueChanged (int)), _part_widget, SLOT (update_widgets()));
		harmonics_layout->addWidget (slider, 0, i);

		QLabel* label = new QLabel (QString ("%1").arg (i + 1), harmonics_grid);
		label->setAlignment (Qt::AlignCenter);
		label->setFixedWidth (kHarmonicsButtonWidth);
		label->setTextFormat (Qt::PlainText);
		harmonics_layout->addWidget (label, 1, i);

		QPushButton* reset = new QPushButton ("R", harmonics_grid);
		reset->setFixedWidth (kHarmonicsButtonWidth);
		reset->setFixedHeight (kHarmonicsButtonHeight);
		reset->setToolTip ("Reset");
		QObject::connect (reset, SIGNAL (clicked()), slider, SLOT (reset()));
		harmonics_layout->addWidget (reset, 2, i);

		_harmonics_sliders.push_back (slider);
		_harmonics_resets.push_back (reset);
	}
	QHBoxLayout* harmonics_tab_layout = new QHBoxLayout (harmonics_widget);
	harmonics_tab_layout->setMargin (0);
	harmonics_tab_layout->setSpacing (0);
	harmonics_tab_layout->addWidget (harmonics_grid);

	// Phase sliders:
	QWidget* harmonic_phases_widget = new QWidget (this);
	harmonic_phases_widget->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QWidget* phases_grid = new QWidget (harmonic_phases_widget);
	QGridLayout* phases_layout = new QGridLayout (phases_grid);
	phases_layout->setSpacing (0);
	for (Sliders::size_type i = 0; i < Params::Part::HarmonicsNumber; ++i)
	{
		harmonics_layout->setColumnMinimumWidth (i, 0);

		Slider* slider = new Slider (Params::Part::PhaseMin, Params::Part::PhaseMax, Params::Part::PhaseMax / 20, Params::Part::PhaseDefault, Qt::Vertical, phases_grid);
		slider->setTracking (true);
		QObject::connect (slider, SIGNAL (valueChanged (int)), _part_widget, SLOT (widgets_to_wave_params()));
		QObject::connect (slider, SIGNAL (valueChanged (int)), _part_widget, SLOT (update_widgets()));
		phases_layout->addWidget (slider, 0, i);

		QLabel* label = new QLabel (QString ("%1").arg (i + 1), phases_grid);
		label->setAlignment (Qt::AlignCenter);
		label->setFixedWidth (kHarmonicsButtonWidth);
		label->setTextFormat (Qt::PlainText);
		phases_layout->addWidget (label, 1, i);

		QPushButton* reset = new QPushButton ("R", phases_grid);
		reset->setFixedWidth (kHarmonicsButtonWidth);
		reset->setFixedHeight (kHarmonicsButtonHeight);
		reset->setToolTip ("Reset");
		phases_layout->addWidget (reset, 2, i);

		QObject::connect (reset, SIGNAL (clicked()), slider, SLOT (reset()));

		_harmonic_phases_sliders.push_back (slider);
		_harmonic_phases_resets.push_back (reset);
	}
	QHBoxLayout* phases_tab_layout = new QHBoxLayout (harmonic_phases_widget);
	phases_tab_layout->setMargin (0);
	phases_tab_layout->setSpacing (0);
	phases_tab_layout->addWidget (phases_grid);

	// Layouts:

	QLabel* harmonics_label = new QLabel ("Harmonics", this);
	// Force normal text color. For some reason Qt uses white color on light-gray background.
	harmonics_label->setForegroundRole (QPalette::Text);
	QLabel* harmonic_phases_label = new QLabel ("Phases", this);
	// Force normal text color. For some reason Qt uses white color on light-gray background.
	harmonic_phases_label->setForegroundRole (QPalette::Text);

	QHBoxLayout* harmonics_panel_buttons_layout = new QHBoxLayout();
	harmonics_panel_buttons_layout->setMargin (0);
	harmonics_panel_buttons_layout->setSpacing (Config::spacing());
	harmonics_panel_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	harmonics_panel_buttons_layout->addWidget (harmonics_reset_button);

	QVBoxLayout* harmonics_panel_layout = new QVBoxLayout (this);
	harmonics_panel_layout->setMargin (Config::margin());
	harmonics_panel_layout->setSpacing (Config::spacing());
	harmonics_panel_layout->addWidget (new Haruhi::StyledBackground (harmonics_label, this, 0.3_screen_mm));
	harmonics_panel_layout->addWidget (harmonics_widget);
	harmonics_panel_layout->addWidget (new Haruhi::StyledBackground (harmonic_phases_label, this, 0.3_screen_mm));
	harmonics_panel_layout->addWidget (harmonic_phases_widget);
	harmonics_panel_layout->addLayout (harmonics_panel_buttons_layout);

	// Save standard button colors:
	_std_button_bg = _harmonics_resets[0]->palette().color (QPalette::Button);
	_std_button_fg = _harmonics_resets[0]->palette().color (QPalette::ButtonText);
}


void
PartHarmonicsWidget::widgets_to_wave_params()
{
	Params::Part* pp = _part->part_params();

	// Update params that are not controller by knobs:
	for (std::size_t i = 0; i < countof (pp->harmonics); ++i)
		pp->harmonics[i].set (_harmonics_sliders[i]->value());
	for (std::size_t i = 0; i < countof (pp->harmonic_phases); ++i)
		pp->harmonic_phases[i].set (_harmonic_phases_sliders[i]->value());
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
	Params::Part* pp = _part->part_params();

	for (std::size_t i = 0; i < countof (pp->harmonics); ++i)
		_harmonics_sliders[i]->setValue (pp->harmonics[i]);
	for (std::size_t i = 0; i < countof (pp->harmonic_phases); ++i)
		_harmonic_phases_sliders[i]->setValue (pp->harmonic_phases[i]);
}

} // namespace Yuki

