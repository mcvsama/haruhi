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
#include <haruhi/widgets/wave_plot.h>
#include <haruhi/widgets/styled_background.h>
#include <haruhi/dsp/modulated_wave.h>
#include <haruhi/dsp/translated_wave.h>
#include <haruhi/dsp/scaled_wave.h>

// Local:
#include "part_widget.h"
#include "params.h"
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
	// Knobs:

	Part::PartControllerProxies* proxies = _part->proxies();

	_knob_wave_shape				= std::make_unique<Haruhi::Knob> (this, &proxies->wave_shape, "Shape");
	_knob_modulator_amplitude		= std::make_unique<Haruhi::Knob> (this, &proxies->modulator_amplitude, "Mod.amp.");
	_knob_modulator_index			= std::make_unique<Haruhi::Knob> (this, &proxies->modulator_index, "Mod.index");
	_knob_modulator_shape			= std::make_unique<Haruhi::Knob> (this, &proxies->modulator_shape, "Mod.shape");
	_knob_volume					= std::make_unique<Haruhi::Knob> (this, &proxies->volume, "Volume dB");
	_knob_panorama					= std::make_unique<Haruhi::Knob> (this, &proxies->panorama, "Panorama");
	_knob_detune					= std::make_unique<Haruhi::Knob> (this, &proxies->detune, "Detune");
	_knob_pitchbend					= std::make_unique<Haruhi::Knob> (this, &proxies->pitchbend, "Pitch");
	_knob_unison_index				= std::make_unique<Haruhi::Knob> (this, &proxies->unison_index, "Unison");
	_knob_unison_spread				= std::make_unique<Haruhi::Knob> (this, &proxies->unison_spread, "U.spread");
	_knob_unison_init				= std::make_unique<Haruhi::Knob> (this, &proxies->unison_init, "U.init.φ");
	_knob_unison_noise				= std::make_unique<Haruhi::Knob> (this, &proxies->unison_noise, "U.noise");
	_knob_unison_vibrato_level		= std::make_unique<Haruhi::Knob> (this, &proxies->unison_vibrato_level, "U.V.lev");
	_knob_unison_vibrato_frequency	= std::make_unique<Haruhi::Knob> (this, &proxies->unison_vibrato_frequency, "U.V.freq");
	_knob_velocity_sens				= std::make_unique<Haruhi::Knob> (this, &proxies->velocity_sens, "Vel.sens.");
	_knob_portamento_time			= std::make_unique<Haruhi::Knob> (this, &proxies->portamento_time, "Glide");
	_knob_phase						= std::make_unique<Haruhi::Knob> (this, &proxies->phase, "Phase");
	_knob_noise_level				= std::make_unique<Haruhi::Knob> (this, &proxies->noise_level, "Noise lvl");

	QObject::connect (_knob_phase.get(), SIGNAL (changed (int)), this, SLOT (update_phase_marker()));

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

	// Top widget, can be disabled with all child widgets:
	_oscillator_panel = new QWidget (this);
	_oscillator_panel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

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
	_modulator_type->insertItem (DSP::ModulatedWave::Ring, Resources::Icons16::modulator_ring(), "Ring pseudo-modulation");
	_modulator_type->insertItem (DSP::ModulatedWave::Frequency, Resources::Icons16::modulator_fm(), "FM pseudo-modulation");
	_modulator_type->setCurrentIndex (pp->modulator_type);
	_modulator_type->setToolTip ("Modulator type");
	_modulator_type->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_modulator_type, SIGNAL (activated (int)), this, SLOT (widgets_to_wave_params()));

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
	_modulator_panel = new PartModulatorWidget (this, this, _part);
	_modulator_panel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	// Harmonics+phases panel:
	_harmonics_panel = new PartHarmonicsWidget (this, this, _part);
	_harmonics_panel->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

	// Tabs:
	_tabs = new QTabWidget (this);
	_tabs->setIconSize (Resources::Icons16::haruhi().size() * 1.25);
	_tabs->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_tabs->addTab (_oscillator_panel, Resources::Icons16::oscillator(), "Oscillator");
	_tabs->addTab (_modulator_panel, Resources::Icons16::modulator(), "Modulator");
	_tabs->addTab (_harmonics_panel, Resources::Icons16::harmonics(), "Harmonics");

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

	QGridLayout* oscillator_panel_layout = new QGridLayout(_oscillator_panel);
	oscillator_panel_layout->setMargin (Config::margin());
	oscillator_panel_layout->setSpacing (Config::spacing());
	oscillator_panel_layout->addWidget (_wave_type, 0, 0, 1, 2);
	oscillator_panel_layout->addWidget (_modulator_wave_type, 0, 2, 1, 2);
	oscillator_panel_layout->addWidget (_modulator_type, 0, 4, 1, 3);
	oscillator_panel_layout->addWidget (_wave_enabled, 0, 7);
	oscillator_panel_layout->addWidget (_noise_enabled, 0, 8);
	oscillator_panel_layout->addWidget (group2, 1, 10, 3, 1);
	oscillator_panel_layout->addWidget (base_plot_frame, 1, 0, 1, 2);
	oscillator_panel_layout->addWidget (final_plot_frame, 1, 2, 1, 2);
	oscillator_panel_layout->addWidget (_knob_volume.get(), 1, 4);
	oscillator_panel_layout->addWidget (_knob_panorama.get(), 1, 5);
	oscillator_panel_layout->addWidget (_knob_velocity_sens.get(), 1, 6);
	oscillator_panel_layout->addWidget (_knob_noise_level.get(), 1, 7);
	oscillator_panel_layout->addWidget (_knob_detune.get(), 1, 8);
	oscillator_panel_layout->addWidget (_knob_pitchbend.get(), 1, 9);
	oscillator_panel_layout->addWidget (_knob_unison_vibrato_level.get(), 2, 8);
	oscillator_panel_layout->addWidget (_knob_unison_vibrato_frequency.get(), 2, 9);
	oscillator_panel_layout->addWidget (_knob_wave_shape.get(), 2, 0);
	oscillator_panel_layout->addWidget (_knob_modulator_amplitude.get(), 2, 1);
	oscillator_panel_layout->addWidget (_knob_modulator_index.get(), 2, 2);
	oscillator_panel_layout->addWidget (_knob_modulator_shape.get(), 2, 3);
	oscillator_panel_layout->addWidget (_knob_unison_index.get(), 2, 4);
	oscillator_panel_layout->addWidget (_knob_unison_spread.get(), 2, 5);
	oscillator_panel_layout->addWidget (_knob_unison_init.get(), 2, 6);
	oscillator_panel_layout->addWidget (_knob_unison_noise.get(), 2, 7);
	oscillator_panel_layout->addWidget (_filter_1, 3, 0, 2, 4);
	oscillator_panel_layout->addWidget (_filter_2, 3, 4, 2, 4);
	oscillator_panel_layout->addWidget (_knob_portamento_time.get(), 3, 8);
	oscillator_panel_layout->addWidget (_knob_phase.get(), 3, 9);
	oscillator_panel_layout->addWidget (group1, 4, 8, 1, 3);

	QWidget* top_checkboxes = new QWidget (this);

	QHBoxLayout* top_checkboxes_layout = new QHBoxLayout (top_checkboxes);
	top_checkboxes_layout->setMargin (0);
	top_checkboxes_layout->setSpacing (2 * Config::spacing());
	top_checkboxes_layout->addWidget (_part_enabled);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::margin());
	layout->setSpacing (Config::spacing());
	layout->addWidget (new Haruhi::StyledBackground (top_checkboxes, this));
	layout->addWidget (_tabs);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

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

	Params::Part* pp = _part->part_params();

	pp->part_enabled = _part_enabled->isChecked();
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

	_modulator_panel->widgets_to_oscillator_params();

	_stop_params_to_widgets = false;
}


void
PartWidget::update_phase_marker()
{
	float pos = 0.5f * (1.0f + _part->part_params()->phase.to_f());
	_base_wave_plot->set_phase_marker_position (pos);
	_base_wave_plot->plot_shape();
	_final_wave_plot->set_phase_marker_position (pos);
	_final_wave_plot->plot_shape();

	_harmonics_panel->update_phase_marker (pos);
}


void
PartWidget::update_widgets()
{
	bool enabled = _part->part_params()->part_enabled.get();
	_oscillator_panel->setEnabled (enabled);
	_modulator_panel->setEnabled (enabled);

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

	_harmonics_panel->setEnabled (enabled && wave_enabled && immutable);

	_harmonics_panel->update_widgets();
	_modulator_panel->update_widgets();
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

	_part_enabled->setChecked (pp->part_enabled);
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

	_base_wave_plot->assign_wave (base_wave, false, true, false);
	_final_wave_plot->assign_wave (_cached_final_wave.get(), false, true, false);

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

