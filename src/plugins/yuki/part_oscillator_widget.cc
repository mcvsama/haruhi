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
#include <QGroupBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/widgets/styled_background.h>
#include <haruhi/widgets/plot_frame.h>
#include <haruhi/widgets/wave_plot.h>
#include <haruhi/dsp/modulated_wave.h>

// Local:
#include "part_oscillator_widget.h"
#include "part_widget.h"
#include "part.h"
#include "part_manager.h"


namespace Yuki {

namespace DSP = Haruhi::DSP;
using namespace Haruhi::ScreenLiterals;


PartOscillatorWidget::PartOscillatorWidget (QWidget* parent, PartWidget* part_widget, Part* part):
	QWidget (parent),
	_part_widget (part_widget),
	_part (part)
{
	// Knobs:

	auto proxies = _part->proxies();

	_knob_wave_shape				= std::make_unique<Haruhi::Knob> (this, &proxies->wave_shape, "Shape");
	_knob_modulator_amplitude		= std::make_unique<Haruhi::Knob> (this, &proxies->modulator_amplitude, "Modulator");
	_knob_modulator_index			= std::make_unique<Haruhi::Knob> (this, &proxies->modulator_index, "M.index");
	_knob_modulator_shape			= std::make_unique<Haruhi::Knob> (this, &proxies->modulator_shape, "M.shape");
	_knob_volume					= std::make_unique<Haruhi::Knob> (this, &proxies->volume, "Level dB");
	_knob_panorama					= std::make_unique<Haruhi::Knob> (this, &proxies->panorama, "Panorama");
	_knob_detune					= std::make_unique<Haruhi::Knob> (this, &proxies->detune, "Detune");
	_knob_pitchbend					= std::make_unique<Haruhi::Knob> (this, &proxies->pitchbend, "Pitch");
	_knob_unison_index				= std::make_unique<Haruhi::Knob> (this, &proxies->unison_index, "Unison");
	_knob_unison_spread				= std::make_unique<Haruhi::Knob> (this, &proxies->unison_spread, "Spread");
	_knob_unison_init				= std::make_unique<Haruhi::Knob> (this, &proxies->unison_init, "Init.φ");
	_knob_unison_noise				= std::make_unique<Haruhi::Knob> (this, &proxies->unison_noise, "Noise");
	_knob_unison_vibrato_level		= std::make_unique<Haruhi::Knob> (this, &proxies->unison_vibrato_level, "Vibrato");
	_knob_unison_vibrato_frequency	= std::make_unique<Haruhi::Knob> (this, &proxies->unison_vibrato_frequency, "Freq");
	_knob_velocity_sens				= std::make_unique<Haruhi::Knob> (this, &proxies->velocity_sens, "Velocity");
	_knob_portamento_time			= std::make_unique<Haruhi::Knob> (this, &proxies->portamento_time, "Glide");
	_knob_phase						= std::make_unique<Haruhi::Knob> (this, &proxies->phase, "Phase");
	_knob_noise_level				= std::make_unique<Haruhi::Knob> (this, &proxies->noise_level, "U.noise");

	auto neutral_color = QColor (0xcc, 0xcc, 0xcc);
	_knob_portamento_time->set_color_hint (neutral_color);
	_knob_velocity_sens->set_color_hint (neutral_color);
	auto level_color = QColor (0x00, 0xbb, 0x00);
	_knob_volume->set_color_hint (level_color);
	_knob_panorama->set_color_hint (level_color);
	_knob_noise_level->set_color_hint (level_color);
	auto pitch_color = QColor (0x00, 0x6e, 0xff);
	_knob_detune->set_color_hint (pitch_color);
	_knob_pitchbend->set_color_hint (pitch_color);
	auto unison_color = QColor (0xff, 0x29, 0x89);
	_knob_unison_index->set_color_hint (unison_color);
	_knob_unison_spread->set_color_hint (unison_color);
	_knob_unison_init->set_color_hint (unison_color);
	_knob_unison_noise->set_color_hint (unison_color);
	_knob_unison_vibrato_level->set_color_hint (unison_color);
	_knob_unison_vibrato_frequency->set_color_hint (unison_color);
	auto shape_color = QColor (0xee, 0xa7, 0x00);
	_knob_wave_shape->set_color_hint (shape_color);
	_knob_modulator_amplitude->set_color_hint (shape_color);
	_knob_modulator_index->set_color_hint (shape_color);
	_knob_modulator_shape->set_color_hint (shape_color);
	_knob_phase->set_color_hint (shape_color);

	QObject::connect (_knob_phase.get(), SIGNAL (changed (int)), _part_widget, SLOT (update_phase_marker()));

	_knob_volume->set_volume_scale (true, M_E);

	// Set unit bay on all knobs:

	for (auto* k: {
		_knob_wave_shape.get(), _knob_modulator_amplitude.get(), _knob_modulator_index.get(), _knob_modulator_shape.get(),
		_knob_volume.get(), _knob_panorama.get(), _knob_detune.get(), _knob_pitchbend.get(), _knob_unison_index.get(), _knob_unison_spread.get(),
		_knob_unison_init.get(), _knob_unison_noise.get(), _knob_unison_vibrato_level.get(), _knob_unison_vibrato_frequency.get(),
		_knob_velocity_sens.get(), _knob_portamento_time.get(), _knob_phase.get(), _knob_noise_level.get() })
	{
		k->set_unit_bay (_part->part_manager()->plugin()->unit_bay());
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

	_base_wave_plot = std::make_unique<Haruhi::WavePlot> (this);
	_base_wave_plot->set_phase_marker_enabled (true);
	_base_wave_plot->set_closed_ring (true);
	_base_wave_plot->set_filled_wave (true);

	auto base_plot_frame = new Haruhi::PlotFrame (this);
	base_plot_frame->set_widget (_base_wave_plot.get());
	base_plot_frame->setToolTip ("Base wave");

	_final_wave_plot = std::make_unique<Haruhi::WavePlot> (this);
	_final_wave_plot->set_phase_marker_enabled (true);
	_final_wave_plot->set_closed_ring (true);
	_final_wave_plot->set_filled_wave (true);

	auto final_plot_frame = new Haruhi::PlotFrame (this);
	final_plot_frame->set_widget (_final_wave_plot.get());
	final_plot_frame->setToolTip ("Output wave (with harmonics, modulation, auto-scaled, etc.)");

	// Shorthand link to part params:
	auto pp = _part->part_params();

	// Wave type:
	_wave_type = std::make_unique<QComboBox> (this);
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
	QObject::connect (_wave_type.get(), SIGNAL (activated (int)), _part_widget, SLOT (widgets_to_wave_params()));

	// Modulator wave type:
	_modulator_wave_type = std::make_unique<QComboBox> (this);
	_modulator_wave_type->insertItem (0, Resources::Icons16::wave_sine(), "Sine");
	_modulator_wave_type->insertItem (1, Resources::Icons16::wave_triangle(), "Triangle");
	_modulator_wave_type->insertItem (2, Resources::Icons16::wave_square(), "Square");
	_modulator_wave_type->insertItem (3, Resources::Icons16::wave_sawtooth(), "Sawtooth");
	_modulator_wave_type->setCurrentIndex (pp->modulator_wave_type);
	_modulator_wave_type->setToolTip ("Modulator wave");
	_modulator_wave_type->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_modulator_wave_type.get(), SIGNAL (activated (int)), _part_widget, SLOT (widgets_to_wave_params()));

	// Modulator type:
	_modulator_type = std::make_unique<QComboBox> (this);
	_modulator_type->insertItem (DSP::ModulatedWave::Ring, Resources::Icons16::modulator_ring(), "Ring pseudo-modulation");
	_modulator_type->insertItem (DSP::ModulatedWave::Frequency, Resources::Icons16::modulator_fm(), "FM pseudo-modulation");
	_modulator_type->setCurrentIndex (pp->modulator_type);
	_modulator_type->setToolTip ("Modulator type");
	_modulator_type->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_modulator_type.get(), SIGNAL (activated (int)), _part_widget, SLOT (widgets_to_wave_params()));

	// Wave enabled:
	_wave_enabled = std::make_unique<QPushButton> ("Wave", this);
	_wave_enabled->setCheckable (true);
	_wave_enabled->setChecked (pp->wave_enabled);
	_wave_enabled->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Fixed);
	QObject::connect (_wave_enabled.get(), SIGNAL (toggled (bool)), _part_widget, SLOT (widgets_to_oscillator_params()));
	QObject::connect (_wave_enabled.get(), SIGNAL (toggled (bool)), _part_widget, SLOT (update_widgets()));

	// Noise enabled:
	_noise_enabled = std::make_unique<QPushButton> ("Noise", this);
	_noise_enabled->setCheckable (true);
	_noise_enabled->setChecked (pp->noise_enabled);
	_noise_enabled->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Fixed);
	QObject::connect (_noise_enabled.get(), SIGNAL (toggled (bool)), _part_widget, SLOT (widgets_to_oscillator_params()));

	// Unison stereo:
	_unison_stereo = std::make_unique<QPushButton> ("Unison stereo", this);
	_unison_stereo->setCheckable (true);
	_unison_stereo->setChecked (pp->unison_stereo);
	_unison_stereo->setToolTip ("Spreads unison voices across stereo channels.");
	QObject::connect (_unison_stereo.get(), SIGNAL (toggled (bool)), _part_widget, SLOT (widgets_to_oscillator_params()));

	// Pseudo stereo:
	_pseudo_stereo = std::make_unique<QPushButton> ("Pseudo stereo", this);
	_pseudo_stereo->setCheckable (true);
	_pseudo_stereo->setChecked (pp->pseudo_stereo);
	_pseudo_stereo->setToolTip ("Inverts right channel to give pseudo-stereo effect for monophonic voices.");
	QObject::connect (_pseudo_stereo.get(), SIGNAL (toggled (bool)), _part_widget, SLOT (widgets_to_oscillator_params()));

	// Auto-center wave:
	_auto_center = std::make_unique<QPushButton> ("Auto center", this);
	_auto_center->setCheckable (true);
	_auto_center->setChecked (pp->auto_center);
	_auto_center->setToolTip ("Auto center wave around 0 level. Takes more CPU power to update wavetables.");
	QObject::connect (_auto_center.get(), SIGNAL (toggled (bool)), _part_widget, SLOT (widgets_to_wave_params()));

	// Const. glide:
	_const_portamento_time = std::make_unique<QPushButton> ("Const. glide", this);
	_const_portamento_time->setCheckable (true);
	_const_portamento_time->setChecked (pp->const_portamento_time);
	QObject::connect (_const_portamento_time.get(), SIGNAL (toggled (bool)), _part_widget, SLOT (widgets_to_oscillator_params()));

	// Pitchbend down:
	_pitchbend_down_semitones = std::make_unique<QSpinBox> (this);
	_pitchbend_down_semitones->setMinimum (-60);
	_pitchbend_down_semitones->setMaximum (0);
	_pitchbend_down_semitones->setValue (-pp->pitchbend_down_semitones);
	QObject::connect (_pitchbend_down_semitones.get(), SIGNAL (valueChanged (int)), _part_widget, SLOT (widgets_to_oscillator_params()));

	// Pitchbend up:
	_pitchbend_up_semitones = std::make_unique<QSpinBox> (this);
	_pitchbend_up_semitones->setMinimum (0);
	_pitchbend_up_semitones->setMaximum (60);
	_pitchbend_up_semitones->setValue (pp->pitchbend_up_semitones);
	QObject::connect (_pitchbend_up_semitones.get(), SIGNAL (valueChanged (int)), _part_widget, SLOT (widgets_to_oscillator_params()));

	// Pitchbend enabled:
	_pitchbend_enabled = std::make_unique<QPushButton> ("Pitchbend", this);
	_pitchbend_enabled->setCheckable (true);
	_pitchbend_enabled->setChecked (pp->pitchbend_enabled);
	QObject::connect (_pitchbend_enabled.get(), SIGNAL (toggled (bool)), _part_widget, SLOT (widgets_to_oscillator_params()));

	// Transposition:
	_transposition_semitones = std::make_unique<QSpinBox> (this);
	_transposition_semitones->setMinimum (-60);
	_transposition_semitones->setMaximum (60);
	_transposition_semitones->setSuffix (" semitones");
	_transposition_semitones->setValue (pp->transposition_semitones);
	QObject::connect (_transposition_semitones.get(), SIGNAL (valueChanged (int)), _part_widget, SLOT (widgets_to_oscillator_params()));

	// Frequency modulation range:
	_frequency_modulation_range = std::make_unique<QSpinBox> (this);
	_frequency_modulation_range->setMinimum (1);
	_frequency_modulation_range->setMaximum (60);
	_frequency_modulation_range->setSuffix (" semitones");
	_frequency_modulation_range->setValue (pp->frequency_mod_range);
	QObject::connect (_frequency_modulation_range.get(), SIGNAL (valueChanged (int)), _part_widget, SLOT (widgets_to_oscillator_params()));

	// Filters:
	_filter_1 = std::make_unique<FilterWidget> (this, 0, &_part->part_params()->voice.filters[0], _part);
	_filter_2 = std::make_unique<FilterWidget> (this, 1, &_part->part_params()->voice.filters[1], _part);

	// Filters configuration:
	_filter_configuration = std::make_unique<QComboBox> (this);
	_filter_configuration->insertItem (DualFilter::Serial, "Filters: Serial");
	_filter_configuration->insertItem (DualFilter::Parallel, "Filters: Parallel");
	_filter_configuration->setCurrentIndex (pp->filter_configuration);
	_filter_configuration->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_filter_configuration.get(), SIGNAL (activated (int)), _part_widget, SLOT (widgets_to_oscillator_params()));

	// Layouts:

	auto pitchbend_range_layout = new QHBoxLayout();
	pitchbend_range_layout->setMargin (0);
	pitchbend_range_layout->setSpacing (Config::spacing());
	pitchbend_range_layout->addWidget (_pitchbend_down_semitones.get());
	pitchbend_range_layout->addWidget (_pitchbend_up_semitones.get());

	auto group1 = new QGroupBox (this);
	auto group1_layout = new QGridLayout (group1);
	group1_layout->setMargin (2 * Config::margin());
	group1_layout->setSpacing (Config::spacing());
	group1_layout->addWidget (new QLabel ("Filters:", this), 0, 0);
	group1_layout->addWidget (_filter_configuration.get(), 0, 1);
	group1_layout->addWidget (new QLabel ("Pitchbend range:", this), 1, 0);
	group1_layout->addLayout (pitchbend_range_layout, 1, 1);
	group1_layout->addWidget (new QLabel ("Freq. mod. range:", this), 2, 0);
	group1_layout->addWidget (_frequency_modulation_range.get(), 2, 1);
	group1_layout->addWidget (new QLabel ("Transposition:", this), 3, 0);
	group1_layout->addWidget (_transposition_semitones.get(), 3, 1);

	auto group2 = new QWidget (this);
	auto group2_layout = new QVBoxLayout (group2);
	group2_layout->setMargin (0);
	group2_layout->setSpacing (Config::spacing());
	group2_layout->addWidget (_auto_center.get());
	group2_layout->addItem (new QSpacerItem (0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
	group2_layout->addWidget (_const_portamento_time.get());
	group2_layout->addWidget (_unison_stereo.get());
	group2_layout->addWidget (_pseudo_stereo.get());
	group2_layout->addItem (new QSpacerItem (0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
	group2_layout->addWidget (_pitchbend_enabled.get());
	group2_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	auto oscillator_panel_layout = new QGridLayout (this);
	oscillator_panel_layout->setMargin (Config::margin());
	oscillator_panel_layout->setSpacing (Config::spacing());
	oscillator_panel_layout->addWidget (_wave_type.get(), 0, 0, 1, 2);
	oscillator_panel_layout->addWidget (_modulator_wave_type.get(), 0, 2, 1, 2);
	oscillator_panel_layout->addWidget (_modulator_type.get(), 0, 4, 1, 3);
	oscillator_panel_layout->addWidget (_wave_enabled.get(), 0, 7);
	oscillator_panel_layout->addWidget (_noise_enabled.get(), 0, 8);
	oscillator_panel_layout->addWidget (group2, 1, 10, 3, 1);
	oscillator_panel_layout->addWidget (base_plot_frame, 1, 0, 1, 2);
	oscillator_panel_layout->addWidget (final_plot_frame, 1, 2, 1, 2);
	oscillator_panel_layout->addWidget (_knob_volume.get(), 1, 4);
	oscillator_panel_layout->addWidget (_knob_noise_level.get(), 1, 5);
	oscillator_panel_layout->addWidget (_knob_panorama.get(), 1, 6);
	oscillator_panel_layout->addWidget (_knob_velocity_sens.get(), 1, 7);
	oscillator_panel_layout->addWidget (_knob_detune.get(), 1, 8);
	oscillator_panel_layout->addWidget (_knob_pitchbend.get(), 1, 9);
	oscillator_panel_layout->addWidget (_knob_wave_shape.get(), 2, 0);
	oscillator_panel_layout->addWidget (_knob_modulator_amplitude.get(), 2, 1);
	oscillator_panel_layout->addWidget (_knob_modulator_index.get(), 2, 2);
	oscillator_panel_layout->addWidget (_knob_modulator_shape.get(), 2, 3);
	oscillator_panel_layout->addWidget (_knob_phase.get(), 2, 4);
	oscillator_panel_layout->addWidget (_knob_portamento_time.get(), 2, 5);
	oscillator_panel_layout->addWidget (_knob_unison_index.get(), 2, 6);
	oscillator_panel_layout->addWidget (_knob_unison_spread.get(), 2, 7);
	oscillator_panel_layout->addWidget (_knob_unison_init.get(), 2, 8);
	oscillator_panel_layout->addWidget (_knob_unison_noise.get(), 2, 9);
	oscillator_panel_layout->addWidget (_knob_unison_vibrato_level.get(), 3, 8);
	oscillator_panel_layout->addWidget (_knob_unison_vibrato_frequency.get(), 3, 9);
	oscillator_panel_layout->addWidget (_filter_1.get(), 3, 0, 2, 4);
	oscillator_panel_layout->addWidget (_filter_2.get(), 3, 4, 2, 4);
	oscillator_panel_layout->addWidget (group1, 4, 8, 1, 3);

	// Listen on certain params changes for updating widgets when necessary:
	pp->wave_shape.on_change.connect (_part_widget, &PartWidget::post_update_wave_plots);
	pp->modulator_amplitude.on_change.connect (_part_widget, &PartWidget::post_update_wave_plots);
	pp->modulator_index.on_change.connect (_part_widget, &PartWidget::post_update_wave_plots);
	pp->modulator_shape.on_change.connect (_part_widget, &PartWidget::post_update_wave_plots);
	// Note: no need to connect to observe harmonics params (and update harmonics sliders),
	// since they will be updated upon showing harmonics window anyway.
	pp->part_enabled.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->wave_enabled.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->noise_enabled.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->frequency_mod_range.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->pitchbend_enabled.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->pitchbend_up_semitones.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->pitchbend_down_semitones.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->transposition_semitones.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->const_portamento_time.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->unison_stereo.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->pseudo_stereo.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->wave_type.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->modulator_type.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->modulator_wave_type.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->auto_center.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
	pp->filter_configuration.on_change.connect (_part_widget, &PartWidget::post_params_to_widgets);
}


bool
PartOscillatorWidget::is_wave_enabled() const
{
	return _wave_enabled->isChecked();
}


void
PartOscillatorWidget::widgets_to_wave_params()
{
	auto pp = _part->part_params();

	// Update params that are not controller by knobs:
	pp->wave_type = _wave_type->currentIndex();
	pp->modulator_type = _modulator_type->currentIndex();
	pp->modulator_wave_type = _modulator_wave_type->currentIndex();
	pp->auto_center = _auto_center->isChecked();
}


void
PartOscillatorWidget::widgets_to_oscillator_params()
{
	auto pp = _part->part_params();

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
}


void
PartOscillatorWidget::update_widgets()
{
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
}


void
PartOscillatorWidget::params_to_widgets()
{
	auto pp = _part->part_params();

	_wave_type->setCurrentIndex (pp->wave_type);
	_modulator_type->setCurrentIndex (pp->modulator_type);
	_modulator_wave_type->setCurrentIndex (pp->modulator_wave_type);
	_auto_center->setChecked (pp->auto_center);

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
}


void
PartOscillatorWidget::update_phase_marker (float pos)
{
	_base_wave_plot->set_phase_marker_position (pos);
	_base_wave_plot->post_plot_shape();
	_final_wave_plot->set_phase_marker_position (pos);
	_final_wave_plot->post_plot_shape();
}


void
PartOscillatorWidget::update_wave_plots (DSP::Wave* base_wave, DSP::Wave* final_wave)
{
	_base_wave_plot->assign_wave (base_wave, false, true, false);
	_final_wave_plot->assign_wave (final_wave, false, true, false);
}

} // namespace Yuki

