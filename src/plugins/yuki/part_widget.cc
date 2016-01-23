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
#include "part_widget.h"
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
	_cached_final_wave (0),
	_stop_widgets_to_params (false),
	_stop_params_to_widgets (false)
{
	// Knobs:

	Part::PartControllerProxies* proxies = _part->proxies();

	_knob_wave_shape				= new Haruhi::Knob (this, &proxies->wave_shape, "Shape");
	_knob_modulator_amplitude		= new Haruhi::Knob (this, &proxies->modulator_amplitude, "Mod.amp.");
	_knob_modulator_index			= new Haruhi::Knob (this, &proxies->modulator_index, "Mod.index");
	_knob_modulator_shape			= new Haruhi::Knob (this, &proxies->modulator_shape, "Mod.shape");
	_knob_volume					= new Haruhi::Knob (this, &proxies->volume, "Volume dB");
	_knob_panorama					= new Haruhi::Knob (this, &proxies->panorama, "Panorama");
	_knob_detune					= new Haruhi::Knob (this, &proxies->detune, "Detune");
	_knob_pitchbend					= new Haruhi::Knob (this, &proxies->pitchbend, "Pitch");
	_knob_unison_index				= new Haruhi::Knob (this, &proxies->unison_index, "Unison");
	_knob_unison_spread				= new Haruhi::Knob (this, &proxies->unison_spread, "U.spread");
	_knob_unison_init				= new Haruhi::Knob (this, &proxies->unison_init, "U.init.φ");
	_knob_unison_noise				= new Haruhi::Knob (this, &proxies->unison_noise, "U.noise");
	_knob_unison_vibrato_level		= new Haruhi::Knob (this, &proxies->unison_vibrato_level, "U.V.lev");
	_knob_unison_vibrato_frequency	= new Haruhi::Knob (this, &proxies->unison_vibrato_frequency, "U.V.freq");
	_knob_velocity_sens				= new Haruhi::Knob (this, &proxies->velocity_sens, "Vel.sens.");
	_knob_portamento_time			= new Haruhi::Knob (this, &proxies->portamento_time, "Glide");
	_knob_phase						= new Haruhi::Knob (this, &proxies->phase, "Phase");
	_knob_noise_level				= new Haruhi::Knob (this, &proxies->noise_level, "Noise lvl");

	// "+ 1" is for output to main oscillator:
	_fm_matrix_knobs.resize (Params::Part::OperatorsNumber + 1);
	_am_matrix_knobs.resize (Params::Part::OperatorsNumber + 1);
	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
	{
		_fm_matrix_knobs[o].resize (Params::Part::OperatorsNumber);
		_am_matrix_knobs[o].resize (Params::Part::OperatorsNumber);
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
		{
			QString t = QString ("%1").arg (o + 1);
			if (o == 3)
				t = "M";
			_fm_matrix_knobs[o][i] = new Haruhi::Knob (this, proxies->fm_matrix[o][i], QString ("%1 → %2").arg (i + 1).arg (t));
			_am_matrix_knobs[o][i] = new Haruhi::Knob (this, proxies->am_matrix[o][i], QString ("%1 → %2").arg (i + 1).arg (t));
			_fm_matrix_knobs[o][i]->set_narrow (true);
			_am_matrix_knobs[o][i]->set_narrow (true);
		}
	}

	QObject::connect (_knob_phase, SIGNAL (changed (int)), this, SLOT (update_phase_marker()));

	_knob_volume->set_volume_scale (true, M_E);

	// Set unit bay on all knobs:

	for (auto* k: {
		_knob_wave_shape, _knob_modulator_amplitude, _knob_modulator_index, _knob_modulator_shape,
		_knob_volume, _knob_panorama, _knob_detune, _knob_pitchbend, _knob_unison_index, _knob_unison_spread,
		_knob_unison_init, _knob_unison_noise, _knob_unison_vibrato_level, _knob_unison_vibrato_frequency,
		_knob_velocity_sens, _knob_portamento_time, _knob_phase, _knob_noise_level })
	{
		k->set_unit_bay (_part->part_manager()->plugin()->unit_bay());
	}

	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
	{
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
		{
			_fm_matrix_knobs[o][i]->set_unit_bay (_part->part_manager()->plugin()->unit_bay());
			_am_matrix_knobs[o][i]->set_unit_bay (_part->part_manager()->plugin()->unit_bay());
		}
	}

	// Help tooltips:

	_knob_unison_index->setToolTip ("Number of voices playing in unison");
	_knob_unison_spread->setToolTip ("Unison frequencies spread");
	_knob_unison_init->setToolTip ("Unison initial phases spread");
	_knob_unison_noise->setToolTip ("Unison noise");
	_knob_unison_vibrato_level->setToolTip ("Unison vibrato level");
	_knob_unison_vibrato_frequency->setToolTip ("Unison vibrato frequency");
	_knob_velocity_sens->setToolTip ("Velocity sensitivity (-1 for reverse, 0 for none)");

	// Wave plots:

	_base_wave_plot = new Haruhi::WavePlot (this);
	_base_wave_plot->set_phase_marker_enabled (true);
	_base_wave_plot->set_closed_ring (true);
	_base_wave_plot->set_filled_wave (true);

	Haruhi::PlotFrame* base_plot_frame = new Haruhi::PlotFrame (this);
	base_plot_frame->set_widget (_base_wave_plot);
	base_plot_frame->setToolTip ("Base wave");

	_final_wave_plot = new Haruhi::WavePlot (this);
	_final_wave_plot->set_phase_marker_enabled (true);
	_final_wave_plot->set_closed_ring (true);
	_final_wave_plot->set_filled_wave (true);

	Haruhi::PlotFrame* final_plot_frame = new Haruhi::PlotFrame (this);
	final_plot_frame->set_widget (_final_wave_plot);
	final_plot_frame->setToolTip ("Output wave (with harmonics, modulation, auto-scaled, etc.)");

	// Shorthand link to part params:
	Params::Part* pp = _part->part_params();

	// Part enabled:
	_part_enabled = new QCheckBox ("Enabled", this);
	_part_enabled->setChecked (pp->part_enabled);
	QObject::connect (_part_enabled, SIGNAL (toggled (bool)), this, SLOT (widgets_to_oscillator_params()));
	QObject::connect (_part_enabled, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	// Modulator enabled:
	_modulator_enabled = new QCheckBox ("Modulator enabled", this);
	_modulator_enabled->setChecked (pp->modulator_enabled);
	QObject::connect (_modulator_enabled, SIGNAL (toggled (bool)), this, SLOT (widgets_to_oscillator_params()));
	QObject::connect (_modulator_enabled, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	// Show modulator checkbox:
	_show_modulator = new QCheckBox ("Show modulator", this);
	_show_modulator->setChecked (false);
	QObject::connect (_show_modulator, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	// Top widget, can be disabled with all child widgets:
	_main_panel = new QWidget (this);
	_main_panel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	// Wave type:
	_wave_type = new QComboBox (this);
	_wave_type->insertItem (0, Resources::Icons16::wave_sine(), "Sine");
	_wave_type->insertItem (1, Resources::Icons16::wave_triangle(), "Triangle");
	_wave_type->insertItem (2, Resources::Icons16::wave_square(), "Square");
	_wave_type->insertItem (3, Resources::Icons16::wave_sawtooth(), "Sawtooth");
	_wave_type->insertItem (4, Resources::Icons16::wave_pulse(), "Pulse");
	_wave_type->insertItem (5, Resources::Icons16::wave_power(), "Power");
	_wave_type->insertItem (6, Resources::Icons16::wave_gauss(), "Gauss");
	_wave_type->insertItem (7, Resources::Icons16::wave_diode(), "Diode");
	_wave_type->insertItem (8, Resources::Icons16::wave_chirp(), "Chirp");
	_wave_type->setIconSize (Resources::Icons16::haruhi().size());
	_wave_type->setCurrentIndex (pp->wave_type);
	QObject::connect (_wave_type, SIGNAL (activated (int)), this, SLOT (widgets_to_wave_params()));

	// Modulator wave type:
	_modulator_wave_type = new QComboBox (this);
	_modulator_wave_type->insertItem (0, Resources::Icons16::wave_sine(), "Sine");
	_modulator_wave_type->insertItem (1, Resources::Icons16::wave_triangle(), "Triangle");
	_modulator_wave_type->insertItem (2, Resources::Icons16::wave_square(), "Square");
	_modulator_wave_type->insertItem (3, Resources::Icons16::wave_sawtooth(), "Sawtooth");
	_modulator_wave_type->setCurrentIndex (pp->modulator_wave_type);
	_modulator_wave_type->setToolTip ("Modulator wave");
	_modulator_wave_type->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_modulator_wave_type, SIGNAL (activated (int)), this, SLOT (widgets_to_wave_params()));

	// Modulator type:
	_modulator_type = new QComboBox (this);
	_modulator_type->insertItem (DSP::ModulatedWave::Ring, Resources::Icons16::modulator_ring(), "Ring mod.");
	_modulator_type->insertItem (DSP::ModulatedWave::Frequency, Resources::Icons16::modulator_fm(), "FM mod.");
	_modulator_type->setCurrentIndex (pp->modulator_type);
	_modulator_type->setToolTip ("Modulator type");
	_modulator_type->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_modulator_type, SIGNAL (activated (int)), this, SLOT (widgets_to_wave_params()));

	// Harmonics+phases window:
	_harmonics_window = new QDialog (this);
	_harmonics_window->hide();
	_harmonics_window->setWindowTitle (QString ("Part %1: Harmonics & Phases").arg (_part->id()));
	_harmonics_window->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

	// Reset all harmonics:
	_harmonics_reset_button = new QPushButton (Resources::Icons16::clear_list(), "Clear all harmonics and phases", _harmonics_window);
	QObject::connect (_harmonics_reset_button, SIGNAL (clicked()), this, SLOT (reset_all_harmonics()));

	// Close harmonics button:
	QPushButton* close_harmonics_button = new QPushButton (Resources::Icons16::dialog_ok(), "Close", _harmonics_window);
	close_harmonics_button->setDefault (true);
	QObject::connect (close_harmonics_button, SIGNAL (clicked()), _harmonics_window, SLOT (accept()));

	// Harmonics & Phases:
	int const kHarmonicsButtonWidth = 5_screen_mm;
	int const kHarmonicsButtonHeight = 4_screen_mm;
	// Harmonics:
	_harmonics_widget = new QWidget (_harmonics_window);
	_harmonics_widget->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QWidget* harmonics_grid = new QWidget (_harmonics_widget);
	QGridLayout* harmonics_layout = new QGridLayout (harmonics_grid);
	harmonics_layout->setSpacing (0);
	for (Sliders::size_type i = 0; i < Params::Part::HarmonicsNumber; ++i)
	{
		harmonics_layout->setColumnMinimumWidth (i, 0);

		int def = i == 0 ? Params::Part::HarmonicMax : Params::Part::HarmonicDefault;
		Slider* slider = new Slider (Params::Part::HarmonicMin, Params::Part::HarmonicMax, Params::Part::HarmonicMax / 20, def, Qt::Vertical, harmonics_grid);
		slider->setTracking (true);
		QObject::connect (slider, SIGNAL (valueChanged (int)), this, SLOT (widgets_to_wave_params()));
		QObject::connect (slider, SIGNAL (valueChanged (int)), this, SLOT (update_widgets()));
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
	QHBoxLayout* harmonics_tab_layout = new QHBoxLayout (_harmonics_widget);
	harmonics_tab_layout->setMargin (0);
	harmonics_tab_layout->setSpacing (0);
	harmonics_tab_layout->addWidget (harmonics_grid);

	// Phases:
	_harmonic_phases_widget = new QWidget (_harmonics_window);
	_harmonic_phases_widget->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QWidget* phases_grid = new QWidget (_harmonic_phases_widget);
	QGridLayout* phases_layout = new QGridLayout (phases_grid);
	phases_layout->setSpacing (0);
	for (Sliders::size_type i = 0; i < Params::Part::HarmonicsNumber; ++i)
	{
		harmonics_layout->setColumnMinimumWidth (i, 0);

		Slider* slider = new Slider (Params::Part::PhaseMin, Params::Part::PhaseMax, Params::Part::PhaseMax / 20, Params::Part::PhaseDefault, Qt::Vertical, phases_grid);
		slider->setTracking (true);
		QObject::connect (slider, SIGNAL (valueChanged (int)), this, SLOT (widgets_to_wave_params()));
		QObject::connect (slider, SIGNAL (valueChanged (int)), this, SLOT (update_widgets()));
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
	QHBoxLayout* phases_tab_layout = new QHBoxLayout (_harmonic_phases_widget);
	phases_tab_layout->setMargin (0);
	phases_tab_layout->setSpacing (0);
	phases_tab_layout->addWidget (phases_grid);

	// Unison stereo:
	_unison_stereo = new QPushButton ("Unison stereo", this);
	_unison_stereo->setCheckable (true);
	_unison_stereo->setChecked (pp->unison_stereo);
	_unison_stereo->setToolTip ("Spreads unison voices across stereo channels.");
	QObject::connect (_unison_stereo, SIGNAL (toggled (bool)), this, SLOT (widgets_to_oscillator_params()));

	// Pseudo stereo:
	_pseudo_stereo = new QPushButton ("Pseudo stereo", this);
	_pseudo_stereo->setCheckable (true);
	_pseudo_stereo->setChecked (pp->pseudo_stereo);
	_pseudo_stereo->setToolTip ("Inverts right channel to give pseudo-stereo effect for monophonic voices.");
	QObject::connect (_pseudo_stereo, SIGNAL (toggled (bool)), this, SLOT (widgets_to_oscillator_params()));

	// Transposition:
	_transposition_semitones = new QSpinBox (this);
	_transposition_semitones->setMinimum (-60);
	_transposition_semitones->setMaximum (60);
	_transposition_semitones->setSuffix (" semitones");
	_transposition_semitones->setValue (pp->transposition_semitones);
	QObject::connect (_transposition_semitones, SIGNAL (valueChanged (int)), this, SLOT (widgets_to_oscillator_params()));

	// Auto-center wave:
	_auto_center = new QPushButton ("Auto center", this);
	_auto_center->setCheckable (true);
	_auto_center->setChecked (pp->auto_center);
	_auto_center->setToolTip ("Auto center wave around 0 level. Takes more CPU power to update wavetables.");
	QObject::connect (_auto_center, SIGNAL (toggled (bool)), this, SLOT (widgets_to_wave_params()));

	// Const. glide:
	_const_portamento_time = new QPushButton ("Const. glide", this);
	_const_portamento_time->setCheckable (true);
	_const_portamento_time->setChecked (pp->const_portamento_time);
	QObject::connect (_const_portamento_time, SIGNAL (toggled (bool)), this, SLOT (widgets_to_oscillator_params()));

	// Pitchbend down:
	_pitchbend_down_semitones = new QSpinBox (this);
	_pitchbend_down_semitones->setMinimum (-60);
	_pitchbend_down_semitones->setMaximum (0);
	_pitchbend_down_semitones->setValue (-pp->pitchbend_down_semitones);
	QObject::connect (_pitchbend_down_semitones, SIGNAL (valueChanged (int)), this, SLOT (widgets_to_oscillator_params()));

	// Pitchbend up:
	_pitchbend_up_semitones = new QSpinBox (this);
	_pitchbend_up_semitones->setMinimum (0);
	_pitchbend_up_semitones->setMaximum (60);
	_pitchbend_up_semitones->setValue (pp->pitchbend_up_semitones);
	QObject::connect (_pitchbend_up_semitones, SIGNAL (valueChanged (int)), this, SLOT (widgets_to_oscillator_params()));

	// Pitchbend enabled:
	_pitchbend_enabled = new QPushButton ("Pitchbend", this);
	_pitchbend_enabled->setCheckable (true);
	_pitchbend_enabled->setChecked (pp->pitchbend_enabled);
	QObject::connect (_pitchbend_enabled, SIGNAL (toggled (bool)), this, SLOT (widgets_to_oscillator_params()));

	// Frequency modulation range:
	_frequency_modulation_range = new QSpinBox (this);
	_frequency_modulation_range->setMinimum (1);
	_frequency_modulation_range->setMaximum (60);
	_frequency_modulation_range->setSuffix (" semitones");
	_frequency_modulation_range->setValue (pp->frequency_mod_range);
	QObject::connect (_frequency_modulation_range, SIGNAL (valueChanged (int)), this, SLOT (widgets_to_oscillator_params()));

	// Wave enabled:
	_wave_enabled = new QPushButton ("Wave", this);
	_wave_enabled->setCheckable (true);
	_wave_enabled->setChecked (pp->wave_enabled);
	_wave_enabled->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Fixed);
	QObject::connect (_wave_enabled, SIGNAL (toggled (bool)), this, SLOT (widgets_to_oscillator_params()));
	QObject::connect (_wave_enabled, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	// Noise enabled:
	_noise_enabled = new QPushButton ("Noise", this);
	_noise_enabled->setCheckable (true);
	_noise_enabled->setChecked (pp->noise_enabled);
	_noise_enabled->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Fixed);
	QObject::connect (_noise_enabled, SIGNAL (toggled (bool)), this, SLOT (widgets_to_oscillator_params()));

	// Show harmonics button:
	QPushButton* show_harmonics = new QPushButton ("Harmonics…", this);
	show_harmonics->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Fixed);
	QObject::connect (show_harmonics, SIGNAL (clicked()), this, SLOT (show_harmonics()));

	// Filters:
	_filter_1 = new FilterWidget (this, 0, &_part->part_params()->voice.filters[0], _part);
	_filter_2 = new FilterWidget (this, 1, &_part->part_params()->voice.filters[1], _part);

	// Filters configuration:
	_filter_configuration = new QComboBox (this);
	_filter_configuration->insertItem (DualFilter::Serial, "Filters: Serial");
	_filter_configuration->insertItem (DualFilter::Parallel, "Filters: Parallel");
	_filter_configuration->setCurrentIndex (pp->filter_configuration);
	_filter_configuration->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_filter_configuration, SIGNAL (activated (int)), this, SLOT (widgets_to_oscillator_params()));

	// Modulator top widget:
	_modulator_panel = new QWidget (this);
	_modulator_panel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	_operator_1 = new OperatorWidget (this, 0, &_part->part_params()->operators[0], _part);
	_operator_2 = new OperatorWidget (this, 1, &_part->part_params()->operators[1], _part);
	_operator_3 = new OperatorWidget (this, 2, &_part->part_params()->operators[2], _part);

	// Stack:
	_stack = new QStackedWidget (this);
	_stack->addWidget (_main_panel);
	_stack->addWidget (_modulator_panel);

	// Layouts:

	QHBoxLayout* pitchbend_range_layout = new QHBoxLayout();
	pitchbend_range_layout->setMargin (0);
	pitchbend_range_layout->setSpacing (Config::spacing());
	pitchbend_range_layout->addWidget (_pitchbend_down_semitones);
	pitchbend_range_layout->addWidget (_pitchbend_up_semitones);

	QGroupBox* group1 = new QGroupBox (this);
	QGridLayout* group1_layout = new QGridLayout (group1);
	group1_layout->setMargin (2 * Config::margin());
	group1_layout->setSpacing (Config::spacing());
	group1_layout->addWidget (new QLabel ("Filters:", this), 0, 0);
	group1_layout->addWidget (_filter_configuration, 0, 1);
	group1_layout->addWidget (new QLabel ("Pitchbend range:", this), 1, 0);
	group1_layout->addLayout (pitchbend_range_layout, 1, 1);
	group1_layout->addWidget (new QLabel ("Freq. mod. range:", this), 2, 0);
	group1_layout->addWidget (_frequency_modulation_range, 2, 1);
	group1_layout->addWidget (new QLabel ("Transposition:", this), 3, 0);
	group1_layout->addWidget (_transposition_semitones, 3, 1);

	QWidget* group2 = new QWidget (this);
	QVBoxLayout* group2_layout = new QVBoxLayout (group2);
	group2_layout->setMargin (0);
	group2_layout->setSpacing (Config::spacing());
	group2_layout->addWidget (_auto_center);
	group2_layout->addItem (new QSpacerItem (0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
	group2_layout->addWidget (_const_portamento_time);
	group2_layout->addWidget (_unison_stereo);
	group2_layout->addWidget (_pseudo_stereo);
	group2_layout->addItem (new QSpacerItem (0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
	group2_layout->addWidget (_pitchbend_enabled);
	group2_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	QGridLayout* panel_layout = new QGridLayout();
	panel_layout->setMargin (0);
	panel_layout->setSpacing (Config::spacing());
	panel_layout->addWidget (_wave_type, 0, 0, 1, 2);
	panel_layout->addWidget (_modulator_wave_type, 0, 2, 1, 2);
	panel_layout->addWidget (_modulator_type, 0, 4, 1, 2);
	panel_layout->addWidget (_wave_enabled, 0, 6);
	panel_layout->addWidget (_noise_enabled, 0, 7);
	panel_layout->addWidget (show_harmonics, 0, 8, 1, 2);
	panel_layout->addWidget (group2, 1, 10, 3, 1);
	panel_layout->addWidget (base_plot_frame, 1, 0, 1, 2);
	panel_layout->addWidget (final_plot_frame, 1, 2, 1, 2);
	panel_layout->addWidget (_knob_volume, 1, 4);
	panel_layout->addWidget (_knob_panorama, 1, 5);
	panel_layout->addWidget (_knob_velocity_sens, 1, 6);
	panel_layout->addWidget (_knob_noise_level, 1, 7);
	panel_layout->addWidget (_knob_detune, 1, 8);
	panel_layout->addWidget (_knob_pitchbend, 1, 9);
	panel_layout->addWidget (_knob_unison_vibrato_level, 2, 8);
	panel_layout->addWidget (_knob_unison_vibrato_frequency, 2, 9);
	panel_layout->addWidget (_knob_wave_shape, 2, 0);
	panel_layout->addWidget (_knob_modulator_amplitude, 2, 1);
	panel_layout->addWidget (_knob_modulator_index, 2, 2);
	panel_layout->addWidget (_knob_modulator_shape, 2, 3);
	panel_layout->addWidget (_knob_unison_index, 2, 4);
	panel_layout->addWidget (_knob_unison_spread, 2, 5);
	panel_layout->addWidget (_knob_unison_init, 2, 6);
	panel_layout->addWidget (_knob_unison_noise, 2, 7);
	panel_layout->addWidget (_filter_1, 3, 0, 2, 4);
	panel_layout->addWidget (_filter_2, 3, 4, 2, 4);
	panel_layout->addWidget (_knob_portamento_time, 3, 8);
	panel_layout->addWidget (_knob_phase, 3, 9);
	panel_layout->addWidget (group1, 4, 8, 1, 3);

	QGridLayout* resizeable_main_layout = new QGridLayout (_main_panel);
	resizeable_main_layout->setMargin (0);
	resizeable_main_layout->setSpacing (0);
	resizeable_main_layout->addLayout (panel_layout, 0, 0);
	resizeable_main_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
	resizeable_main_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 0);

	QLabel* fm_modulation_matrix_label = create_modulator_label ("FM modulation matrix:");
	QLabel* am_modulation_matrix_label = create_modulator_label ("AM modulation matrix:");
	fm_modulation_matrix_label->setFixedHeight (2.2f * Haruhi::Services::y_pixels_per_point() * fm_modulation_matrix_label->font().pointSize());
	am_modulation_matrix_label->setFixedHeight (2.2f * Haruhi::Services::y_pixels_per_point() * fm_modulation_matrix_label->font().pointSize());

	QGridLayout* modulator_layout = new QGridLayout (_modulator_panel);
	modulator_layout->setMargin (0);
	modulator_layout->setSpacing (Config::spacing());
	modulator_layout->addWidget (_modulator_enabled, 0, 0);
	modulator_layout->addWidget (_operator_1, 1, 0);
	modulator_layout->addWidget (_operator_2, 2, 0);
	modulator_layout->addWidget (_operator_3, 3, 0);
	modulator_layout->addWidget (create_modulator_label ("→"), 1, 1, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("→"), 2, 1, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("→"), 3, 1, 1, 1, Qt::AlignCenter);
	modulator_layout->addItem (new QSpacerItem (Config::spacing() * 4, 0, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 1);
	modulator_layout->addWidget (fm_modulation_matrix_label, 0, 2, 1, 4, Qt::AlignLeft);
	modulator_layout->addItem (new QSpacerItem (Config::spacing() * 4, 0, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 6);
	modulator_layout->addWidget (am_modulation_matrix_label, 0, 7, 1, 4, Qt::AlignLeft);
	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
			modulator_layout->addWidget (_fm_matrix_knobs[o][i], i + 1, o + 2);
	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
			modulator_layout->addWidget (_am_matrix_knobs[o][i], i + 1, o + 7);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 1"), 4, 2, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 2"), 4, 3, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 3"), 4, 4, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op M"), 4, 5, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 1"), 4, 7, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 2"), 4, 8, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 3"), 4, 9, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op M"), 4, 10, 1, 1, Qt::AlignCenter);

	modulator_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 5, 0);
	modulator_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 11);

	QWidget* top_checkboxes = new QWidget (this);

	QHBoxLayout* top_checkboxes_layout = new QHBoxLayout (top_checkboxes);
	top_checkboxes_layout->setMargin (0);
	top_checkboxes_layout->setSpacing (2 * Config::spacing());
	top_checkboxes_layout->addWidget (_part_enabled);
	top_checkboxes_layout->addWidget (_show_modulator);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::margin());
	layout->setSpacing (Config::spacing());
	layout->addWidget (new Haruhi::StyledBackground (top_checkboxes, this));
	layout->addWidget (_stack);

	QLabel* harmonics_label = new QLabel ("Harmonics", _harmonics_window);
	// Force normal text color. For some reason Qt uses white color on light-gray background.
	harmonics_label->setForegroundRole (QPalette::Text);
	QLabel* harmonic_phases_label = new QLabel ("Phases", _harmonics_window);
	// Force normal text color. For some reason Qt uses white color on light-gray background.
	harmonic_phases_label->setForegroundRole (QPalette::Text);

	QHBoxLayout* harmonics_window_buttons_layout = new QHBoxLayout();
	harmonics_window_buttons_layout->setMargin (0);
	harmonics_window_buttons_layout->setSpacing (Config::spacing());
	harmonics_window_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	harmonics_window_buttons_layout->addWidget (close_harmonics_button);
	harmonics_window_buttons_layout->addWidget (_harmonics_reset_button);

	QVBoxLayout* harmonics_window_layout = new QVBoxLayout (_harmonics_window);
	harmonics_window_layout->setMargin (Config::window_margin());
	harmonics_window_layout->setSpacing (Config::spacing());
	harmonics_window_layout->addWidget (new Haruhi::StyledBackground (harmonics_label, _harmonics_window, 0.4_screen_mm));
	harmonics_window_layout->addWidget (_harmonics_widget);
	harmonics_window_layout->addWidget (new Haruhi::StyledBackground (harmonic_phases_label, _harmonics_window, 0.4_screen_mm));
	harmonics_window_layout->addWidget (_harmonic_phases_widget);
	harmonics_window_layout->addLayout (harmonics_window_buttons_layout);

	// Save standard button colors:
	_std_button_bg = _harmonics_resets[0]->palette().color (QPalette::Button);
	_std_button_fg = _harmonics_resets[0]->palette().color (QPalette::ButtonText);

	widgets_to_wave_params();
	update_widgets();

	// Listen on certain params changes for updating widgets when necessary:
	pp->wave_shape.on_change.connect (this, &PartWidget::post_update_wave_plots);
	pp->modulator_amplitude.on_change.connect (this, &PartWidget::post_update_wave_plots);
	pp->modulator_index.on_change.connect (this, &PartWidget::post_update_wave_plots);
	pp->modulator_shape.on_change.connect (this, &PartWidget::post_update_wave_plots);
	// Note: no need to connect to observe harmonics params (and update harmonics sliders),
	// since they will be updated upon showing harmonics window anyway.
	pp->part_enabled.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->wave_enabled.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->noise_enabled.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->frequency_mod_range.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->pitchbend_enabled.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->pitchbend_up_semitones.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->pitchbend_down_semitones.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->transposition_semitones.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->const_portamento_time.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->unison_stereo.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->pseudo_stereo.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->wave_type.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->modulator_type.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->modulator_wave_type.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->auto_center.on_change.connect (this, &PartWidget::post_params_to_widgets);
	pp->filter_configuration.on_change.connect (this, &PartWidget::post_params_to_widgets);
}


PartWidget::~PartWidget()
{
	delete _knob_wave_shape;
	delete _knob_modulator_amplitude;
	delete _knob_modulator_index;
	delete _knob_modulator_shape;
	delete _knob_volume;
	delete _knob_panorama;
	delete _knob_detune;
	delete _knob_pitchbend;
	delete _knob_velocity_sens;
	delete _knob_unison_index;
	delete _knob_unison_spread;
	delete _knob_unison_init;
	delete _knob_unison_noise;
	delete _knob_unison_vibrato_level;
	delete _knob_unison_vibrato_frequency;
	delete _knob_portamento_time;
	delete _knob_phase;
	delete _knob_noise_level;

	delete _cached_final_wave;
}


void
PartWidget::widgets_to_wave_params()
{
	if (_stop_widgets_to_params)
		return;
	_stop_params_to_widgets = true;

	Params::Part* pp = _part->part_params();

	// Update params that are not controller by knobs:
	pp->wave_type = _wave_type->currentIndex();
	pp->modulator_type = _modulator_type->currentIndex();
	pp->modulator_wave_type = _modulator_wave_type->currentIndex();
	pp->auto_center = _auto_center->isChecked();
	for (std::size_t i = 0; i < countof (pp->harmonics); ++i)
		pp->harmonics[i].set (_harmonics_sliders[i]->value());
	for (std::size_t i = 0; i < countof (pp->harmonic_phases); ++i)
		pp->harmonic_phases[i].set (_harmonic_phases_sliders[i]->value());

	update_wave_plots();

	_stop_params_to_widgets = false;
}


void
PartWidget::widgets_to_oscillator_params()
{
	if (_stop_widgets_to_params)
		return;
	_stop_params_to_widgets = true;

	Params::Part* pp = _part->part_params();

	pp->part_enabled = _part_enabled->isChecked();
	pp->modulator_enabled = _modulator_enabled->isChecked();
	pp->wave_enabled = _wave_enabled->isChecked();
	pp->noise_enabled = _noise_enabled->isChecked();
	pp->frequency_mod_range = _frequency_modulation_range->value();
	pp->pitchbend_enabled = _pitchbend_enabled->isChecked();
	pp->pitchbend_up_semitones = _pitchbend_up_semitones->value();
	pp->pitchbend_down_semitones = -_pitchbend_down_semitones->value();
	pp->transposition_semitones = _transposition_semitones->value();
	pp->const_portamento_time = _const_portamento_time->isChecked();
	pp->unison_stereo = _unison_stereo->isChecked();
	pp->pseudo_stereo = _pseudo_stereo->isChecked();
	pp->filter_configuration = _filter_configuration->currentIndex();

	_stop_params_to_widgets = false;
}


void
PartWidget::show_harmonics()
{
	_harmonics_window->show();
	_harmonics_window->activateWindow();
}


void
PartWidget::update_phase_marker()
{
	float pos = 0.5f * (1.0f + _part->part_params()->phase.to_f());
	_base_wave_plot->set_phase_marker_position (pos);
	_base_wave_plot->plot_shape();
	_final_wave_plot->set_phase_marker_position (pos);
	_final_wave_plot->plot_shape();
}


void
PartWidget::reset_all_harmonics()
{
	for (QSlider* s: _harmonics_sliders)
		static_cast<Slider*> (s)->reset();
	for (QSlider* s: _harmonic_phases_sliders)
		static_cast<Slider*> (s)->reset();
}


void
PartWidget::update_widgets()
{
	_main_panel->setEnabled (_part->part_params()->part_enabled.get());
	_modulator_panel->setEnabled (_part->part_params()->part_enabled.get());

	const bool wave_enabled = _wave_enabled->isChecked();
	_base_wave_plot->setEnabled (wave_enabled);
	_final_wave_plot->setEnabled (wave_enabled);

	const bool immutable = _part->base_wave()->immutable();
	_knob_wave_shape->setEnabled (immutable);
	_knob_modulator_amplitude->setEnabled (immutable);
	_knob_modulator_index->setEnabled (immutable);
	_knob_modulator_shape->setEnabled (immutable);
	_modulator_type->setEnabled (immutable);
	_modulator_wave_type->setEnabled (immutable);
	_harmonics_widget->setEnabled (immutable);
	_harmonic_phases_widget->setEnabled (immutable);

	for (Sliders::size_type i = 0; i < Params::Part::HarmonicsNumber; ++i)
	{
		set_button_highlighted (_harmonics_resets[i], _harmonics_sliders[i]->value() != 0);
		set_button_highlighted (_harmonic_phases_resets[i], _harmonic_phases_sliders[i]->value() != 0);
	}

	if (_show_modulator->isChecked())
		_stack->setCurrentWidget (_modulator_panel);
	else
		_stack->setCurrentWidget (_main_panel);

	const bool mod_enabled = _modulator_enabled->isChecked();
	_operator_1->setEnabled (mod_enabled);
	_operator_2->setEnabled (mod_enabled);
	_operator_3->setEnabled (mod_enabled);

	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
	{
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
		{
			_fm_matrix_knobs[o][i]->setEnabled (mod_enabled);
			_am_matrix_knobs[o][i]->setEnabled (mod_enabled);
		}
	}

	for (auto ml: _modulator_labels)
		ml->setEnabled (mod_enabled);
}


void
PartWidget::params_to_widgets()
{
	if (_stop_params_to_widgets)
		return;
	_stop_widgets_to_params = true;

	Params::Part* pp = _part->part_params();

	_wave_type->setCurrentIndex (pp->wave_type);
	_modulator_type->setCurrentIndex (pp->modulator_type);
	_modulator_wave_type->setCurrentIndex (pp->modulator_wave_type);
	_auto_center->setChecked (pp->auto_center);
	for (std::size_t i = 0; i < countof (pp->harmonics); ++i)
		_harmonics_sliders[i]->setValue (pp->harmonics[i]);
	for (std::size_t i = 0; i < countof (pp->harmonic_phases); ++i)
		_harmonic_phases_sliders[i]->setValue (pp->harmonic_phases[i]);

	_part_enabled->setChecked (pp->part_enabled);
	_modulator_enabled->setChecked (pp->modulator_enabled);
	_wave_enabled->setChecked (pp->wave_enabled);
	_noise_enabled->setChecked (pp->noise_enabled);
	_frequency_modulation_range->setValue (pp->frequency_mod_range);
	_pitchbend_enabled->setChecked (pp->pitchbend_enabled);
	_pitchbend_up_semitones->setValue (pp->pitchbend_up_semitones);
	_pitchbend_down_semitones->setValue (-pp->pitchbend_down_semitones);
	_transposition_semitones->setValue (pp->transposition_semitones);
	_const_portamento_time->setChecked (pp->const_portamento_time);
	_unison_stereo->setChecked (pp->unison_stereo);
	_pseudo_stereo->setChecked (pp->pseudo_stereo);
	_filter_configuration->setCurrentIndex (pp->filter_configuration);

	_stop_widgets_to_params = false;

	update_widgets();
}


void
PartWidget::update_wave_plots()
{
	DSP::Wave* previous_final_wave = _cached_final_wave;

	_cached_final_wave = _part->final_wave();

	DSP::Wave* base_wave = _cached_final_wave;
	while (base_wave->inner_wave())
		base_wave = base_wave->inner_wave();

	_base_wave_plot->assign_wave (base_wave, false, true, false);
	_final_wave_plot->assign_wave (_cached_final_wave, false, true, false);

	// This will also call plot_shape() on plots:
	update_phase_marker();

	delete previous_final_wave;
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


QLabel*
PartWidget::create_modulator_label (QString const& text)
{
	QLabel* label = new QLabel (text, this);
	_modulator_labels.push_back (label);
	return label;
}

} // namespace Yuki

