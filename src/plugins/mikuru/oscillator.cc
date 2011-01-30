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
#include <string>
#include <numeric>

// Qt:
#include <QtGui/QToolTip>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

// Haruhi:
#include <haruhi/widgets/wave_plot.h>

// Local:
#include "mikuru.h"
#include "part.h"
#include "oscillator.h"
#include "params.h"
#include "widgets.h"
#include "wave_computer.h"


namespace MikuruPrivate {

/*
 * Methods for updating individual Voice parameters
 * instead of all parameters at once.
 * TODO create own ControllerProxy that will override set_value and call _part->voice_manager()->set_voice_param (Haruhi::OmniVoice, …)
 */

#define UPDATE_VOICE(param_name) \
	void Oscillator::update_voice_##param_name() \
	{ \
		if (_loading_params) \
			return; \
		_part->voice_manager()->set_voice_param (Haruhi::OmniVoice, &Params::Voice::param_name, _voice_params.param_name.get()); \
	}

UPDATE_VOICE (panorama)
UPDATE_VOICE (detune)
UPDATE_VOICE (pitchbend)
UPDATE_VOICE (unison_index)
UPDATE_VOICE (unison_spread)
UPDATE_VOICE (unison_init)
UPDATE_VOICE (unison_noise)
UPDATE_VOICE (velocity_sens)

#undef UPDATE_VOICE


Oscillator::Oscillator (Part* part, Haruhi::PortGroup* port_group, QString const& q_port_prefix, Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_part (part),
	_loading_params (false)
{
	std::string port_prefix = q_port_prefix.toStdString();

	Params::Waveform pw = _waveform_params;
	Params::Oscillator po = _oscillator_params;
	Params::Voice pv = _voice_params;

	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	_port_wave_shape = new Haruhi::EventPort (_mikuru, port_prefix + " - Wave shape", Haruhi::Port::Input, port_group);
	_port_modulator_amplitude = new Haruhi::EventPort (_mikuru, port_prefix + " - Wave modulator amplitude", Haruhi::Port::Input, port_group);
	_port_modulator_index = new Haruhi::EventPort (_mikuru, port_prefix + " - Wave modulator index", Haruhi::Port::Input, port_group);
	_port_modulator_shape = new Haruhi::EventPort (_mikuru, port_prefix + " - Wave modulator shape", Haruhi::Port::Input, port_group);
	_port_volume = new Haruhi::EventPort (_mikuru, port_prefix + " - Volume", Haruhi::Port::Input, port_group);
	_port_amplitude = new Haruhi::EventPort (_mikuru, port_prefix + " - Amplitude modulation", Haruhi::Port::Input, port_group, Haruhi::Port::Polyphonic);
	_port_frequency = new Haruhi::EventPort (_mikuru, port_prefix + " - Frequency modulation", Haruhi::Port::Input, port_group, Haruhi::Port::Polyphonic);
	_port_panorama = new Haruhi::EventPort (_mikuru, port_prefix + " - Panorama", Haruhi::Port::Input, port_group, Haruhi::Port::Polyphonic);
	_port_detune = new Haruhi::EventPort (_mikuru, port_prefix + " - Detune", Haruhi::Port::Input, port_group, Haruhi::Port::Polyphonic);
	_port_pitchbend = new Haruhi::EventPort (_mikuru, port_prefix + " - Pitchbend", Haruhi::Port::Input, port_group, Haruhi::Port::Polyphonic);
	_port_velocity_sens = new Haruhi::EventPort (_mikuru, port_prefix + " - Velocity sensitivity", Haruhi::Port::Input, port_group, Haruhi::Port::Polyphonic);
	_port_unison_index = new Haruhi::EventPort (_mikuru, port_prefix + " - Unison index", Haruhi::Port::Input, port_group, Haruhi::Port::Polyphonic);
	_port_unison_spread = new Haruhi::EventPort (_mikuru, port_prefix + " - Unison spread", Haruhi::Port::Input, port_group, Haruhi::Port::Polyphonic);
	_port_unison_init = new Haruhi::EventPort (_mikuru, port_prefix + " - Unison init. φ", Haruhi::Port::Input, port_group, Haruhi::Port::Polyphonic);
	_port_unison_noise = new Haruhi::EventPort (_mikuru, port_prefix + " - Unison noise", Haruhi::Port::Input, port_group, Haruhi::Port::Polyphonic);
	_port_portamento_time = new Haruhi::EventPort (_mikuru, port_prefix + " - Portamento time", Haruhi::Port::Input, port_group);
	_port_phase = new Haruhi::EventPort (_mikuru, port_prefix + " - Phase", Haruhi::Port::Input, port_group);
	_port_noise_level = new Haruhi::EventPort (_mikuru, port_prefix + " - Noise level", Haruhi::Port::Input, port_group);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();

	// Knobs:

	_knob_wave_shape			= new Haruhi::Knob (this, _port_wave_shape, &_waveform_params.wave_shape, "Shape",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Waveform::WaveShape, 100), 2);
	_knob_modulator_amplitude	= new Haruhi::Knob (this, _port_modulator_amplitude, &_waveform_params.modulator_amplitude, "Mod.amp.",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Waveform::ModulatorAmplitude, 100), 2);
	_knob_modulator_index		= new Haruhi::Knob (this, _port_modulator_index, &_waveform_params.modulator_index, "Mod.index",
													HARUHI_MIKURU_PARAMS_FOR_KNOB (Params::Waveform::ModulatorIndex), 1, 0);
	_knob_modulator_shape		= new Haruhi::Knob (this, _port_modulator_shape, &_waveform_params.modulator_shape, "Mod.shape",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Waveform::ModulatorShape, 100), 2);
	_knob_volume				= new Haruhi::Knob (this, _port_volume, &_oscillator_params.volume, "Volume dB",
													-std::numeric_limits<float>::infinity(), 0.0f,
													(Params::Oscillator::VolumeMax - Params::Oscillator::VolumeMin) / 500, 2);
	_knob_panorama				= new Haruhi::Knob (this, _port_panorama, &_voice_params.panorama, "Panorama",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::Panorama, 100), 2);
	_knob_detune				= new Haruhi::Knob (this, _port_detune, &_voice_params.detune, "Detune",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::Detune, 100), 2);
	_knob_pitchbend				= new Haruhi::Knob (this, _port_pitchbend, &_voice_params.pitchbend, "Pitch",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::Pitchbend, 100), 2);
	_knob_unison_index			= new Haruhi::Knob (this, _port_unison_index, &_voice_params.unison_index, "Unison",
													HARUHI_MIKURU_PARAMS_FOR_KNOB (Params::Voice::UnisonIndex), 1, 0);
	_knob_unison_spread			= new Haruhi::Knob (this, _port_unison_spread, &_voice_params.unison_spread, "U.spread",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::UnisonSpread, 100), 2);
	_knob_unison_init			= new Haruhi::Knob (this, _port_unison_init, &_voice_params.unison_init, "U.init.φ",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::UnisonInit, 200), 2);
	_knob_unison_noise			= new Haruhi::Knob (this, _port_unison_noise, &_voice_params.unison_noise, "U.noise",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::UnisonNoise, 100), 2);
	_knob_velocity_sens			= new Haruhi::Knob (this, _port_velocity_sens, &_voice_params.velocity_sens, "Vel.sens.",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::VelocitySens, 100), 2);
	_knob_portamento_time		= new Haruhi::Knob (this, _port_portamento_time, &_oscillator_params.portamento_time, "Glide",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Oscillator::PortamentoTime, 100), 2);
	_knob_phase					= new Haruhi::Knob (this, _port_phase, &_oscillator_params.phase, "Phase",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Oscillator::Phase, 200), 2);
	_knob_noise_level			= new Haruhi::Knob (this, _port_noise_level, &_oscillator_params.noise_level, "Noise lvl",
													HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Oscillator::NoiseLevel, 200), 2);

	// Set unit bay:
	Haruhi::Knob* all_knobs[] = {
		_knob_wave_shape, _knob_modulator_amplitude, _knob_modulator_index, _knob_modulator_shape,
		_knob_volume, _knob_panorama, _knob_detune, _knob_pitchbend, _knob_unison_index, _knob_unison_spread,
		_knob_unison_init, _knob_unison_noise, _knob_velocity_sens, _knob_portamento_time, _knob_phase, _knob_noise_level
	};
	for (Haruhi::Knob** k = all_knobs; k != all_knobs+ sizeof (all_knobs) / sizeof (*all_knobs); ++k)
		(*k)->set_unit_bay (_mikuru->unit_bay());

	_knob_portamento_time->controller_proxy().config().curve = 1.0;
	_knob_portamento_time->controller_proxy().config().user_limit_max = 0.5f * Params::Oscillator::PortamentoTimeDenominator;
	_knob_portamento_time->controller_proxy().apply_config();

	_knob_volume->set_volume_scale (true, M_E);

	QObject::connect (_knob_wave_shape, SIGNAL (changed (int)), this, SLOT (recompute_wave()));
	QObject::connect (_knob_modulator_amplitude, SIGNAL (changed (int)), this, SLOT (recompute_wave()));
	QObject::connect (_knob_modulator_index, SIGNAL (changed (int)), this, SLOT (recompute_wave()));
	QObject::connect (_knob_modulator_shape, SIGNAL (changed (int)), this, SLOT (recompute_wave()));
	QObject::connect (_knob_panorama, SIGNAL (changed (int)), this, SLOT (update_voice_panorama()));
	QObject::connect (_knob_detune, SIGNAL (changed (int)), this, SLOT (update_voice_detune()));
	QObject::connect (_knob_pitchbend, SIGNAL (changed (int)), this, SLOT (update_voice_pitchbend()));
	QObject::connect (_knob_unison_index, SIGNAL (changed (int)), this, SLOT (update_voice_unison_index()));
	QObject::connect (_knob_unison_spread, SIGNAL (changed (int)), this, SLOT (update_voice_unison_spread()));
	QObject::connect (_knob_unison_init, SIGNAL (changed (int)), this, SLOT (update_voice_unison_init()));
	QObject::connect (_knob_unison_noise, SIGNAL (changed (int)), this, SLOT (update_voice_unison_noise()));
	QObject::connect (_knob_velocity_sens, SIGNAL (changed (int)), this, SLOT (update_voice_velocity_sens()));

	QToolTip::add (_knob_unison_index, "Number of voices playing in unison");
	QToolTip::add (_knob_unison_spread, "Unison frequencies spread");
	QToolTip::add (_knob_unison_init, "Unison initial phases spread");
	QToolTip::add (_knob_unison_noise, "Unison noise");
	QToolTip::add (_knob_velocity_sens, "Velocity sensitivity (-1 for reverse, 0 for none)");

	// EventDispatchers for polyphonic controls:

	VoiceManager* vm = _part->voice_manager();
	_evdisp_amplitude = new EventDispatcher (_port_amplitude, Params::Voice::AmplitudeMin, Params::Voice::AmplitudeMax, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::amplitude));
	_evdisp_frequency = new EventDispatcher (_port_frequency, Params::Voice::FrequencyMin, Params::Voice::FrequencyMax, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::frequency));
	_evdisp_panorama = new EventDispatcher (_port_panorama, _knob_panorama, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::panorama));
	_evdisp_detune = new EventDispatcher (_port_detune, _knob_detune, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::detune));
	_evdisp_pitchbend = new EventDispatcher (_port_pitchbend, _knob_pitchbend, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::pitchbend));
	_evdisp_velocity_sens = new EventDispatcher (_port_velocity_sens, _knob_velocity_sens, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::velocity_sens));
	_evdisp_unison_index = new EventDispatcher (_port_unison_index, _knob_unison_index, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::unison_index));
	_evdisp_unison_spread = new EventDispatcher (_port_unison_spread, _knob_unison_spread, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::unison_spread));
	_evdisp_unison_init = new EventDispatcher (_port_unison_init, _knob_unison_init, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::unison_init));
	_evdisp_unison_noise = new EventDispatcher (_port_unison_noise, _knob_unison_noise, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::unison_noise));

	// Wave types:

	_waves.push_back (WaveInfo (Resources::Icons16::wave_sine(),		"Sine",		new DSP::ParametricWaves::Sine()));
	_waves.push_back (WaveInfo (Resources::Icons16::wave_triangle(),	"Triangle",	new DSP::ParametricWaves::Triangle()));
	_waves.push_back (WaveInfo (Resources::Icons16::wave_square(),		"Square",	new DSP::ParametricWaves::Square()));
	_waves.push_back (WaveInfo (Resources::Icons16::wave_sawtooth(),	"Sawtooth",	new DSP::ParametricWaves::Sawtooth()));
	_waves.push_back (WaveInfo (Resources::Icons16::wave_pulse(),		"Pulse",	new DSP::ParametricWaves::Pulse()));
	_waves.push_back (WaveInfo (Resources::Icons16::wave_power(),		"Power",	new DSP::ParametricWaves::Power()));
	_waves.push_back (WaveInfo (Resources::Icons16::wave_gauss(),		"Gauss",	new DSP::ParametricWaves::Gauss()));
	_waves.push_back (WaveInfo (Resources::Icons16::wave_diode(),		"Diode",	new DSP::ParametricWaves::Diode()));
	_waves.push_back (WaveInfo (Resources::Icons16::wave_chirp(),		"Chirp",	new DSP::ParametricWaves::Chirp()));

	// Modulator waves:

	_modulator_waves.push_back (WaveInfo (Resources::Icons16::wave_sine(),		"Sine",		new DSP::ParametricWaves::Sine()));
	_modulator_waves.push_back (WaveInfo (Resources::Icons16::wave_triangle(),	"Triangle",	new DSP::ParametricWaves::Triangle()));
	_modulator_waves.push_back (WaveInfo (Resources::Icons16::wave_square(),	"Square",	new DSP::ParametricWaves::Square()));
	_modulator_waves.push_back (WaveInfo (Resources::Icons16::wave_sawtooth(),	"Sawtooth",	new DSP::ParametricWaves::Sawtooth()));

	_wave_computer = new WaveComputer();
	_wave_computer->finished.connect (this, &Oscillator::update_wave_plot);

	// Wave plots:

	QFrame* base_plot_frame = new QFrame (this);
	base_plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	base_plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_base_wave_plot = new Haruhi::WavePlot (base_plot_frame);
	QToolTip::add (_base_wave_plot, "Base wave");
	QVBoxLayout* base_plot_frame_layout = new QVBoxLayout (base_plot_frame, 0, Config::Spacing);
	base_plot_frame_layout->addWidget (_base_wave_plot);

	QFrame* harmonics_plot_frame = new QFrame (this);
	harmonics_plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	harmonics_plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_final_wave_plot = new Haruhi::WavePlot (harmonics_plot_frame);
	QToolTip::add (_final_wave_plot, "Wave with harmonics and modulation");
	QVBoxLayout* harmonics_plot_frame_layout = new QVBoxLayout (harmonics_plot_frame, 0, Config::Spacing);
	harmonics_plot_frame_layout->addWidget (_final_wave_plot);

	// Wave type:

	_wave_type = new QComboBox (this);
	for (Waves::size_type i = 0; i < _waves.size(); ++i)
		_wave_type->insertItem (_waves[i].icon, _waves[i].name, i);
	_wave_type->setCurrentItem (pw.wave_type);
	QObject::connect (_wave_type, SIGNAL (activated (int)), this, SLOT (update_waveform_params()));
	QObject::connect (_wave_type, SIGNAL (activated (int)), this, SLOT (update_widgets()));
	QToolTip::add (_wave_type, "Oscillator wave");

	// Modulator wave type:

	_modulator_wave_type = new QComboBox (this);
	for (Waves::size_type i = 0; i < _modulator_waves.size(); ++i)
		_modulator_wave_type->insertItem (_modulator_waves[i].icon, _modulator_waves[i].name, i);
	_modulator_wave_type->setCurrentItem (pw.modulator_wave_type);
	QObject::connect (_modulator_wave_type, SIGNAL (activated (int)), this, SLOT (update_waveform_params()));
	QToolTip::add (_modulator_wave_type, "Modulator wave");

	// Modulator type:

	_modulator_type = new QComboBox (this);
	_modulator_type->insertItem (Resources::Icons16::modulator_ring(), "Ring mod.", DSP::ModulatedWave::Ring);
	_modulator_type->insertItem (Resources::Icons16::modulator_fm(), "FM mod.", DSP::ModulatedWave::Frequency);
	_modulator_type->setCurrentItem (pw.modulator_type);
	QObject::connect (_modulator_type, SIGNAL (activated (int)), this, SLOT (update_waveform_params()));
	QToolTip::add (_modulator_type, "Modulator type");

	// Harmonics+phases tabs:

	_harmonics_window = new QDialog (this);
	_harmonics_window->hide();
	_harmonics_window->setWindowTitle ("Harmonics & Phases");

	_harmonics_and_phases_tabs = new QTabWidget (_harmonics_window);

	// Harmonics:

	_harmonics_tab = new QWidget (_harmonics_and_phases_tabs);
	_harmonics_tab->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QWidget* harmonics_grid = new QWidget (_harmonics_tab);
	QGridLayout* harmonics_layout = new QGridLayout (harmonics_grid);
	harmonics_layout->setSpacing (0);
	for (Sliders::size_type i = 0; i < Params::Waveform::HarmonicsNumber; ++i)
	{
		harmonics_layout->setColumnMinimumWidth (i, 0);

		int def = i == 0 ? Params::Waveform::HarmonicMax : Params::Waveform::HarmonicDefault;
		Slider* slider = new Slider (Params::Waveform::HarmonicMin, Params::Waveform::HarmonicMax, Params::Waveform::HarmonicMax / 20, def, Qt::Vertical, harmonics_grid);
		slider->setTracking (true);
		QObject::connect (slider, SIGNAL (valueChanged (int)), this, SLOT (sliders_updated()));
		harmonics_layout->addWidget (slider, 0, i);

		QLabel* label = new QLabel (QString ("%1").arg (i + 1), harmonics_grid);
		label->setAlignment (Qt::AlignCenter);
		label->setFixedWidth (23);
		harmonics_layout->addWidget (label, 1, i);

		QPushButton* reset = new QPushButton ("R", harmonics_grid);
		reset->setFixedWidth (23);
		reset->setFixedHeight (18);
		QToolTip::add (reset, "Reset");
		QObject::connect (reset, SIGNAL (clicked()), slider, SLOT (reset()));
		harmonics_layout->addWidget (reset, 2, i);

		_harmonics_sliders.push_back (slider);
		_harmonics_resets.push_back (reset);
	}
	QHBoxLayout* harmonics_tab_layout = new QHBoxLayout (_harmonics_tab, 0, 0);
	harmonics_tab_layout->addWidget (harmonics_grid);

	// Phases:

	_phases_tab = new QWidget (_harmonics_and_phases_tabs);
	_phases_tab->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QWidget* phases_grid = new QWidget (_phases_tab);
	QGridLayout* phases_layout = new QGridLayout (phases_grid);
	phases_layout->setSpacing (0);
	for (Sliders::size_type i = 0; i < Params::Waveform::HarmonicsNumber; ++i)
	{
		harmonics_layout->setColumnMinimumWidth (i, 0);

		Slider* slider = new Slider (Params::Waveform::PhaseMin, Params::Waveform::PhaseMax, Params::Waveform::PhaseMax / 20, Params::Waveform::PhaseDefault, Qt::Vertical, phases_grid);
		slider->setTracking (true);
		QObject::connect (slider, SIGNAL (valueChanged (int)), this, SLOT (sliders_updated()));
		phases_layout->addWidget (slider, 0, i);

		QLabel* label = new QLabel (QString ("%1").arg (i + 1), phases_grid);
		label->setAlignment (Qt::AlignCenter);
		label->setFixedWidth (23);
		phases_layout->addWidget (label, 1, i);

		QPushButton* reset = new QPushButton ("R", phases_grid);
		reset->setFixedWidth (23);
		reset->setFixedHeight (18);
		QToolTip::add (reset, "Reset");
		phases_layout->addWidget (reset, 2, i);

		QObject::connect (reset, SIGNAL (clicked()), slider, SLOT (reset()));

		_phases_sliders.push_back (slider);
		_phases_resets.push_back (reset);
	}
	QHBoxLayout* phases_tab_layout = new QHBoxLayout (_phases_tab, 0, 0);
	phases_tab_layout->addWidget (phases_grid);

	_harmonics_and_phases_tabs->addTab (_harmonics_tab, "Harmonics");
	_harmonics_and_phases_tabs->addTab (_phases_tab, "Phases");

	// Monophonic:
	_monophonic = new QCheckBox ("Mono", this);
	_monophonic->setChecked (po.monophonic);
	QObject::connect (_monophonic, SIGNAL (toggled (bool)), this, SLOT (update_oscillator_params()));
	QObject::connect (_monophonic, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	// Monophonic retrigger:
	_monophonic_retrigger = new QCheckBox ("Mono retrigger", this);
	_monophonic_retrigger->setChecked (po.monophonic_retrigger);
	QObject::connect (_monophonic_retrigger, SIGNAL (toggled (bool)), this, SLOT (update_oscillator_params()));

	// Key priority:
	_monophonic_key_priority = new QComboBox (this);
	_monophonic_key_priority->insertItem ("Last pressed", Params::Oscillator::LastPressed);
	_monophonic_key_priority->insertItem ("First pressed", Params::Oscillator::FirstPressed);
	_monophonic_key_priority->insertItem ("Lowest", Params::Oscillator::LowestPressed);
	_monophonic_key_priority->insertItem ("Highest", Params::Oscillator::HighestPressed);
	_monophonic_key_priority->setCurrentItem (po.monophonic_key_priority);
	QObject::connect (_monophonic_key_priority, SIGNAL (activated (int)), this, SLOT (update_oscillator_params()));

	// Unison stereo:
	_unison_stereo = new QCheckBox ("Unison stereo", this);
	_unison_stereo->setChecked (po.unison_stereo);
	QObject::connect (_unison_stereo, SIGNAL (toggled (bool)), this, SLOT (update_oscillator_params()));
	QToolTip::add (_unison_stereo, "Spreads unison voices across stereo channels.");

	// Pseudo stereo:
	_pseudo_stereo = new QCheckBox ("Pseudo stereo", this);
	_pseudo_stereo->setChecked (po.pseudo_stereo);
	QObject::connect (_pseudo_stereo, SIGNAL (toggled (bool)), this, SLOT (update_oscillator_params()));
	QToolTip::add (_pseudo_stereo, "Inverts right channel to give pseudo-stereo effect for monophonic voices.");

	// Transposition:
	_transposition_semitones = new QSpinBox (-60, 60, 1, this);
	_transposition_semitones->setSuffix (" semitones");
	_transposition_semitones->setValue (po.transposition_semitones);
	QObject::connect (_transposition_semitones, SIGNAL (valueChanged (int)), this, SLOT (update_oscillator_params()));

	// Glide:
	_const_portamento_time = new QCheckBox ("Const. glide", this);
	_const_portamento_time->setChecked (po.const_portamento_time);
	QObject::connect (_const_portamento_time, SIGNAL (toggled (bool)), this, SLOT (update_oscillator_params()));

	// Pitchbend down:
	_pitchbend_down_semitones = new QSpinBox (-60, 0, 1, this);
	_pitchbend_down_semitones->setValue (-po.pitchbend_down_semitones);
	QObject::connect (_pitchbend_down_semitones, SIGNAL (valueChanged (int)), this, SLOT (update_oscillator_params()));

	// Pitchbend up:
	_pitchbend_up_semitones = new QSpinBox (0, 60, 1, this);
	_pitchbend_up_semitones->setValue (po.pitchbend_up_semitones);
	QObject::connect (_pitchbend_up_semitones, SIGNAL (valueChanged (int)), this, SLOT (update_oscillator_params()));

	// Pitchbend enabled:
	_pitchbend_enabled = new QCheckBox ("Pitchbend", this);
	_pitchbend_enabled->setChecked (po.pitchbend_enabled);
	QObject::connect (_pitchbend_enabled, SIGNAL (toggled (bool)), this, SLOT (update_oscillator_params()));

	// Pitchbend released:
	_pitchbend_released = new QCheckBox ("Bend released", this);
	_pitchbend_released->setChecked (po.pitchbend_released);
	QObject::connect (_pitchbend_released, SIGNAL (toggled (bool)), this, SLOT (update_oscillator_params()));
	QToolTip::add (_pitchbend_released, "Alter pitch of already released voices");

	// Frequency modulation range:
	_frequency_modulation_range = new QSpinBox (1, 60, 1, this);
	_frequency_modulation_range->setSuffix (" semitones");
	_frequency_modulation_range->setValue (po.frequency_mod_range);
	QObject::connect (_frequency_modulation_range, SIGNAL (valueChanged (int)), this, SLOT (update_oscillator_params()));

	// Wave enabled:
	_wave_enabled = new QPushButton ("Wave", this);
	_wave_enabled->setCheckable (true);
	_wave_enabled->setChecked (po.wave_enabled);
	_wave_enabled->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Fixed);
	QObject::connect (_wave_enabled, SIGNAL (toggled (bool)), this, SLOT (update_oscillator_params()));
	QObject::connect (_wave_enabled, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	// Noise enabled:
	_noise_enabled = new QPushButton ("Noise", this);
	_noise_enabled->setCheckable (true);
	_noise_enabled->setChecked (po.noise_enabled);
	_noise_enabled->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Fixed);
	QObject::connect (_noise_enabled, SIGNAL (toggled (bool)), this, SLOT (update_oscillator_params()));

	// Show harmonics button:
	QPushButton* show_harmonics = new QPushButton ("Harmonics…", this);
	show_harmonics->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Fixed);
	QObject::connect (show_harmonics, SIGNAL (clicked()), this, SLOT (show_harmonics()));

	// Layouts:

	QHBoxLayout* pitchbend_range_layout = new QHBoxLayout();
	pitchbend_range_layout->setMargin (0);
	pitchbend_range_layout->setSpacing (Config::Spacing);
	pitchbend_range_layout->addWidget (_pitchbend_down_semitones);
	pitchbend_range_layout->addWidget (_pitchbend_up_semitones);

	QGroupBox* group1 = new QGroupBox (this);
	QGridLayout* group1_layout = new QGridLayout (group1);
	group1_layout->setMargin (2 * Config::Margin);
	group1_layout->setSpacing (Config::Spacing);
	group1_layout->addWidget (new QLabel ("Pitchbend range:", this), 0, 0);
	group1_layout->addLayout (pitchbend_range_layout, 0, 1);
	group1_layout->addWidget (new QLabel ("Freq. mod. range:", this), 1, 0);
	group1_layout->addWidget (_frequency_modulation_range, 1, 1);
	group1_layout->addWidget (new QLabel ("Transposition:", this), 2, 0);
	group1_layout->addWidget (_transposition_semitones, 2, 1);

	QGroupBox* group2 = new QGroupBox (this);
	QVBoxLayout* group2_layout = new QVBoxLayout (group2);
	group2_layout->setMargin (2 * Config::Margin);
	group2_layout->setSpacing (Config::Spacing);
	group2_layout->addWidget (_monophonic);
	group2_layout->addWidget (_monophonic_retrigger);
	group2_layout->addItem (new QSpacerItem (0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
	group2_layout->addWidget (new QLabel ("Mono key priority:", this));
	group2_layout->addWidget (_monophonic_key_priority);
	group2_layout->addItem (new QSpacerItem (0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
	group2_layout->addWidget (_const_portamento_time);
	group2_layout->addWidget (_unison_stereo);
	group2_layout->addWidget (_pseudo_stereo);
	group2_layout->addItem (new QSpacerItem (0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
	group2_layout->addWidget (_pitchbend_enabled);
	group2_layout->addWidget (_pitchbend_released);
	group2_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
	group2_layout->addWidget (show_harmonics);

	QGridLayout* layout = new QGridLayout();
	layout->setMargin (Config::Margin);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (_wave_type, 0, 0, 1, 2);
	layout->addWidget (_modulator_wave_type, 0, 2, 1, 2);
	layout->addWidget (_modulator_type, 0, 4, 1, 2);
	layout->addWidget (_wave_enabled, 0, 6);
	layout->addWidget (_noise_enabled, 0, 7);
	layout->addWidget (base_plot_frame, 1, 0, 1, 2);
	layout->addWidget (harmonics_plot_frame, 1, 2, 1, 2);
	layout->addWidget (_knob_volume, 1, 4);
	layout->addWidget (_knob_panorama, 1, 5);
	layout->addWidget (_knob_velocity_sens, 1, 6);
	layout->addWidget (_knob_noise_level, 1, 7);
	layout->addWidget (_knob_wave_shape, 2, 0);
	layout->addWidget (_knob_modulator_amplitude, 2, 1);
	layout->addWidget (_knob_modulator_index, 2, 2);
	layout->addWidget (_knob_modulator_shape, 2, 3);
	layout->addWidget (_knob_detune, 2, 4);
	layout->addWidget (_knob_pitchbend, 2, 5);
	layout->addWidget (_knob_portamento_time, 2, 6);
	layout->addWidget (_knob_phase, 2, 7);
	layout->addWidget (_knob_unison_index, 3, 0);
	layout->addWidget (_knob_unison_spread, 3, 1);
	layout->addWidget (_knob_unison_init, 3, 2);
	layout->addWidget (_knob_unison_noise, 3, 3);
	layout->addWidget (group1, 3, 4, 1, 4);
	layout->addWidget (group2, 0, 8, 4, 1);

	QGridLayout* top_layout = new QGridLayout (this);
	top_layout->setMargin (0);
	top_layout->setSpacing (0);
	top_layout->addLayout (layout, 0, 0);
	top_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
	top_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 0);

	QVBoxLayout* harmonics_window_layout = new QVBoxLayout (_harmonics_window);
	harmonics_window_layout->setMargin (Config::DialogMargin);
	harmonics_window_layout->setSpacing (Config::Spacing);
	harmonics_window_layout->addWidget (_harmonics_and_phases_tabs);

	// Save standard button colors:
	_std_button_bg = _harmonics_resets[0]->paletteBackgroundColor();
	_std_button_fg = _harmonics_resets[0]->paletteForegroundColor();

	update_widgets();
	update_waveform_params();
}


Oscillator::~Oscillator()
{
	// Wave computer might call its finished() signal at any
	// time, and disconnecting is not thread-safe at the moment.
	// So we should try to delete WaveComputer first:
	if (_mikuru->graph())
		_mikuru->graph()->lock();
	delete _wave_computer;
	if (_mikuru->graph())
		_mikuru->graph()->unlock();

	Signal::Receiver::disconnect_all_signals();

	delete _evdisp_amplitude;
	delete _evdisp_frequency;
	delete _evdisp_panorama;
	delete _evdisp_detune;
	delete _evdisp_pitchbend;
	delete _evdisp_velocity_sens;
	delete _evdisp_unison_index;
	delete _evdisp_unison_spread;
	delete _evdisp_unison_init;
	delete _evdisp_unison_noise;

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
	delete _knob_portamento_time;
	delete _knob_phase;
	delete _knob_noise_level;

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	delete _port_wave_shape;
	delete _port_modulator_amplitude;
	delete _port_modulator_index;
	delete _port_modulator_shape;
	delete _port_volume;
	delete _port_amplitude;
	delete _port_frequency;
	delete _port_panorama;
	delete _port_detune;
	delete _port_pitchbend;
	delete _port_velocity_sens;
	delete _port_unison_index;
	delete _port_unison_spread;
	delete _port_unison_init;
	delete _port_unison_noise;
	delete _port_portamento_time;
	delete _port_phase;
	delete _port_noise_level;
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
Oscillator::process_events()
{
	// Waveform:
	_knob_wave_shape->controller_proxy().process_events();
	_knob_modulator_amplitude->controller_proxy().process_events();
	_knob_modulator_index->controller_proxy().process_events();
	_knob_modulator_shape->controller_proxy().process_events();
	// Oscillator:
	_knob_volume->controller_proxy().process_events();
	_knob_portamento_time->controller_proxy().process_events();
	_knob_phase->controller_proxy().process_events();
	_knob_noise_level->controller_proxy().process_events();
	// Voice:
	_evdisp_amplitude->load_events();
	_evdisp_frequency->load_events();
	_evdisp_panorama->load_events();
	_evdisp_detune->load_events();
	_evdisp_pitchbend->load_events();
	_evdisp_velocity_sens->load_events();
	_evdisp_unison_index->load_events();
	_evdisp_unison_spread->load_events();
	_evdisp_unison_init->load_events();
	_evdisp_unison_noise->load_events();
}


DSP::Wavetable*
Oscillator::wavetable()
{
	return _wave_computer->wavetable();
}


void
Oscillator::load_waveform_params()
{
	_loading_params = true;

	Params::Waveform pw = _waveform_params;

	_wave_type->setCurrentItem (pw.wave_type);
	_modulator_type->setCurrentItem (pw.modulator_type);
	_modulator_wave_type->setCurrentItem (pw.modulator_wave_type);
	for (Sliders::size_type i = 0; i < _harmonics_sliders.size(); ++i)
		_harmonics_sliders[i]->setValue (pw.harmonics[i]);
	for (Sliders::size_type i = 0; i < _phases_sliders.size(); ++i)
		_phases_sliders[i]->setValue (pw.phases[i]);

	_loading_params = false;
	update_widgets();
	recompute_wave();
}


void
Oscillator::load_oscillator_params()
{
	_loading_params = true;

	Params::Oscillator po = _oscillator_params;

	// Other:
	_wave_enabled->setChecked (po.wave_enabled);
	_noise_enabled->setChecked (po.noise_enabled);
	_frequency_modulation_range->setValue (po.frequency_mod_range);
	_pitchbend_enabled->setChecked (po.pitchbend_enabled);
	_pitchbend_released->setChecked (po.pitchbend_released);
	_pitchbend_up_semitones->setValue (po.pitchbend_up_semitones);
	_pitchbend_down_semitones->setValue (-po.pitchbend_down_semitones);
	_transposition_semitones->setValue (po.transposition_semitones);
	_monophonic->setChecked (po.monophonic);
	_monophonic_retrigger->setChecked (po.monophonic_retrigger);
	_monophonic_key_priority->setCurrentItem (po.monophonic_key_priority);
	_const_portamento_time->setChecked (po.const_portamento_time);
	_unison_stereo->setChecked (po.unison_stereo);
	_pseudo_stereo->setChecked (po.pseudo_stereo);

	_loading_params = false;
	update_widgets();
}


void
Oscillator::load_voice_params()
{
}


void
Oscillator::load_waveform_params (Params::Waveform& params)
{
	_waveform_params = params;
	load_waveform_params();
}


void
Oscillator::load_oscillator_params (Params::Oscillator& params)
{
	_oscillator_params = params;
	load_oscillator_params();
}


void
Oscillator::load_voice_params (Params::Voice& params)
{
	_voice_params = params;
	load_voice_params();
}


void
Oscillator::update_waveform_params()
{
	if (_loading_params)
		return;

	Params::Waveform pw;

	pw.wave_type = _wave_type->currentItem();
	pw.modulator_type = _modulator_type->currentItem();
	pw.modulator_wave_type = _modulator_wave_type->currentItem();
	for (Sliders::size_type i = 0; i < _harmonics_sliders.size(); ++i)
		pw.harmonics[i] = _harmonics_sliders[i]->value();
	for (Sliders::size_type i = 0; i < _phases_sliders.size(); ++i)
		pw.phases[i] = _phases_sliders[i]->value();

	// Knob params are updated automatically using #assign_parameter.

	_waveform_params.set_non_controller_params (pw);
	recompute_wave();
}


void
Oscillator::update_oscillator_params()
{
	if (_loading_params)
		return;

	Params::Oscillator po;

	po.wave_enabled = _wave_enabled->isChecked();
	po.noise_enabled = _noise_enabled->isChecked();
	po.frequency_mod_range = _frequency_modulation_range->value();
	po.pitchbend_enabled = _pitchbend_enabled->isChecked();
	po.pitchbend_released = _pitchbend_released->isChecked();
	po.pitchbend_up_semitones = _pitchbend_up_semitones->value();
	po.pitchbend_down_semitones = -_pitchbend_down_semitones->value();
	po.transposition_semitones = _transposition_semitones->value();
	po.monophonic = _monophonic->isChecked();
	po.monophonic_retrigger = _monophonic_retrigger->isChecked();
	po.monophonic_key_priority = _monophonic_key_priority->currentItem();
	po.const_portamento_time = _const_portamento_time->isChecked();
	po.unison_stereo = _unison_stereo->isChecked();
	po.pseudo_stereo = _pseudo_stereo->isChecked();

	// Knob params are updated automatically using #assign_parameter.

	_oscillator_params.set_non_controller_params (po);
}


void
Oscillator::update_voice_params()
{
	if (_loading_params)
		return;

	// No non-controller params to set.

	// Send voice params to all existing voices:
	// FIXME problem: it'd be better to update only changed param, not all, because
	// 		 setting other params will mess with fe. params controlled by LFO.
	_part->voice_manager()->set_all_voices_params (_voice_params);
}


void
Oscillator::update_widgets()
{
	bool waves = _wave_enabled->isChecked();
	_base_wave_plot->setEnabled (waves);
	_final_wave_plot->setEnabled (waves);
	_monophonic_key_priority->setEnabled (_monophonic->isChecked());
	// Waveform:
	bool immutable = active_wave().wave.get()->immutable();
	_knob_wave_shape->setEnabled (immutable);
	_knob_modulator_amplitude->setEnabled (immutable);
	_knob_modulator_index->setEnabled (immutable);
	_knob_modulator_shape->setEnabled (immutable);
	_modulator_type->setEnabled (immutable);
	_modulator_wave_type->setEnabled (immutable);
	_harmonics_tab->setEnabled (immutable);
	_phases_tab->setEnabled (immutable);
}


void
Oscillator::recompute_wave()
{
	DSP::ParametricWave* pw = active_wave().wave.get();
	// Continue only if wave is immutable:
	if (pw->immutable())
	{
		DSP::ParametricWave* mw = active_modulator_wave().wave.get();
		pw->set_param (_waveform_params.wave_shape.to_f());
		mw->set_param (_waveform_params.modulator_shape.to_f());
		// Add harmonics to pw:
		DSP::HarmonicsWave* hw = new DSP::HarmonicsWave (pw);
		for (DSP::HarmonicsWave::Harmonics::size_type i = 0; i < hw->harmonics().size(); ++i)
		{
			int hv = _harmonics_sliders[i]->value();
			int pv = _phases_sliders[i]->value();
			float h = 1.0f * hv / Params::Waveform::HarmonicDenominator;
			float p = 1.0f * pv / Params::Waveform::PhaseDenominator;
			// Apply exponential curve to harmonic value:
			h = h > 0 ? std::pow (h, M_E) : -std::pow (-h, M_E);
			hw->set_harmonic (i, h, p);
			set_button_highlighted (_harmonics_resets[i], hv != 0);
			set_button_highlighted (_phases_resets[i], pv != 0);
		}
		// Modulate wave:
		int xt = _waveform_params.modulator_type.get();
		DSP::ModulatedWave* xw = new DSP::ModulatedWave (hw, mw, static_cast<DSP::ModulatedWave::Type> (xt),
														 1.0f * _waveform_params.modulator_amplitude.get() / Params::Waveform::ModulatorAmplitudeMax,
														 _waveform_params.modulator_index.get(), true);
		// Recompute:
		_wave_computer->update (xw);
	}
	else
	{
		_base_wave_plot->assign_wave (pw, false, true);
		_base_wave_plot->post_plot_shape();

		_final_wave_plot->assign_wave (pw, false, true);
		_final_wave_plot->post_plot_shape();
	}
}


void
Oscillator::sliders_updated()
{
	if (!_loading_params)
		update_waveform_params();
}


void
Oscillator::show_harmonics()
{
	_harmonics_window->show();
	_harmonics_window->activateWindow();
}


Oscillator::WaveInfo&
Oscillator::active_wave()
{
	return _waves[std::min (_waveform_params.wave_type.get(), static_cast<unsigned int> (_waves.size() - 1))];
}


Oscillator::WaveInfo&
Oscillator::active_modulator_wave()
{
	return _modulator_waves[std::min (_waveform_params.modulator_wave_type.get(), static_cast<unsigned int> (_modulator_waves.size() - 1))];
}


void
Oscillator::update_wave_plot (Shared<DSP::Wave> const& wave)
{
	// Temporarily prevent plotters to use previous wave,
	// because it will be dropped now:
	_base_wave_plot->assign_wave (0);
	_final_wave_plot->assign_wave (0);

	// Replace wave for plotters (will delete previous waves):
	_plotters_wave = wave;

	DSP::ModulatedWave* modulated_wave = dynamic_cast<DSP::ModulatedWave*> (&*_plotters_wave);
	if (modulated_wave)
	{
		DSP::HarmonicsWave* harmonics_wave = dynamic_cast<DSP::HarmonicsWave*> (modulated_wave->wave());
		if (harmonics_wave)
		{
			DSP::ParametricWave* parametric_wave = dynamic_cast<DSP::ParametricWave*> (harmonics_wave->wave());
			if (parametric_wave)
			{
				// We're not Qt-thread, we must use post_plot_shape.

				_base_wave_plot->assign_wave (parametric_wave, false, true);
				_base_wave_plot->post_plot_shape();

				_final_wave_plot->assign_wave (modulated_wave, false, true);
				_final_wave_plot->post_plot_shape();
			}
		}
	}
}


void
Oscillator::set_button_highlighted (QPushButton* button, bool highlight)
{
	button->setPaletteBackgroundColor (highlight ? QColor (0x00, 0xff, 0x00) : _std_button_bg);
	button->setPaletteForegroundColor (highlight ? QColor (0x00, 0x00, 0x00) : _std_button_fg);
}

} // namespace MikuruPrivate

