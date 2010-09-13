/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
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

// Qt:
#include <QtGui/QToolTip>
#include <Qt3Support/Q3GroupBox>

// Local:
#include "mikuru.h"
#include "part.h"
#include "oscillator.h"
#include "params.h"


namespace MikuruPrivate {

/*
 * Methods for updating individual Voice parameters
 * instead of all parameters at once.
 * TODO create own ControllerProxy that will override set_value and call _part->voice_manager()->set_voice_param (Core::OmniVoice, …)
 */

#define UPDATE_VOICE(param_name) \
	void Oscillator::update_voice_##param_name() \
	{ \
		if (_loading_params) \
			return; \
		_part->voice_manager()->set_voice_param (Core::OmniVoice, &Params::Voice::param_name, _voice_params.param_name); \
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


Oscillator::Oscillator (Part* part, Core::PortGroup* port_group, QString const& q_port_prefix, Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_part (part),
	_loading_params (false)
{
	std::string port_prefix = q_port_prefix.toStdString();

	Params::Oscillator po = _oscillator_params;
	Params::Voice pv = _voice_params;

	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_panel = new QWidget (this);

	_mikuru->graph()->lock();
	_port_volume = new Core::EventPort (_mikuru, port_prefix + " - Volume", Core::Port::Input, port_group);
	_port_amplitude = new Core::EventPort (_mikuru, port_prefix + " - Amplitude modulation", Core::Port::Input, port_group, Core::Port::Polyphonic);
	_port_frequency = new Core::EventPort (_mikuru, port_prefix + " - Frequency modulation", Core::Port::Input, port_group, Core::Port::Polyphonic);
	_port_panorama = new Core::EventPort (_mikuru, port_prefix + " - Panorama", Core::Port::Input, port_group, Core::Port::Polyphonic);
	_port_detune = new Core::EventPort (_mikuru, port_prefix + " - Detune", Core::Port::Input, port_group, Core::Port::Polyphonic);
	_port_pitchbend = new Core::EventPort (_mikuru, port_prefix + " - Pitchbend", Core::Port::Input, port_group, Core::Port::Polyphonic);
	_port_velocity_sens = new Core::EventPort (_mikuru, port_prefix + " - Velocity sensitivity", Core::Port::Input, port_group, Core::Port::Polyphonic);
	_port_unison_index = new Core::EventPort (_mikuru, port_prefix + " - Unison index", Core::Port::Input, port_group, Core::Port::Polyphonic);
	_port_unison_spread = new Core::EventPort (_mikuru, port_prefix + " - Unison spread", Core::Port::Input, port_group, Core::Port::Polyphonic);
	_port_unison_init = new Core::EventPort (_mikuru, port_prefix + " - Unison init. φ", Core::Port::Input, port_group, Core::Port::Polyphonic);
	_port_unison_noise = new Core::EventPort (_mikuru, port_prefix + " - Unison noise", Core::Port::Input, port_group, Core::Port::Polyphonic);
	_port_portamento_time = new Core::EventPort (_mikuru, port_prefix + " - Portamento time", Core::Port::Input, port_group);
	_port_phase = new Core::EventPort (_mikuru, port_prefix + " - Phase", Core::Port::Input, port_group);
	_port_noise_level = new Core::EventPort (_mikuru, port_prefix + " - Noise level", Core::Port::Input, port_group);
	_mikuru->graph()->unlock();

	_proxy_volume = new ControllerProxy (_port_volume, &_oscillator_params.volume, 0, HARUHI_MIKURU_MINMAX (Params::Oscillator::Volume), po.volume);
	_proxy_panorama = new ControllerProxy (_port_panorama, &_voice_params.panorama, &_oscillator_params.panorama_smoothing, HARUHI_MIKURU_MINMAX (Params::Voice::Panorama), pv.panorama);
	_proxy_detune = new ControllerProxy (_port_detune, &_voice_params.detune, 0, HARUHI_MIKURU_MINMAX (Params::Voice::Detune), pv.detune);
	_proxy_pitchbend = new ControllerProxy (_port_pitchbend, &_voice_params.pitchbend, &_oscillator_params.pitchbend_smoothing, HARUHI_MIKURU_MINMAX (Params::Voice::Pitchbend), pv.pitchbend);
	_proxy_unison_index = new ControllerProxy (_port_unison_index, &_voice_params.unison_index, 0, HARUHI_MIKURU_MINMAX (Params::Voice::UnisonIndex), pv.unison_index);
	_proxy_unison_spread = new ControllerProxy (_port_unison_spread, &_voice_params.unison_spread, 0, HARUHI_MIKURU_MINMAX (Params::Voice::UnisonSpread), pv.unison_spread);
	_proxy_unison_init = new ControllerProxy (_port_unison_init, &_voice_params.unison_init, 0, HARUHI_MIKURU_MINMAX (Params::Voice::UnisonInit), pv.unison_init);
	_proxy_unison_noise = new ControllerProxy (_port_unison_noise, &_voice_params.unison_noise, 0, HARUHI_MIKURU_MINMAX (Params::Voice::UnisonNoise), pv.unison_noise);
	_proxy_velocity_sens = new ControllerProxy (_port_velocity_sens, &_voice_params.velocity_sens, 0, HARUHI_MIKURU_MINMAX (Params::Voice::VelocitySens), pv.velocity_sens);
	_proxy_portamento_time = new ControllerProxy (_port_portamento_time, &_oscillator_params.portamento_time, 0, HARUHI_MIKURU_MINMAX (Params::Oscillator::PortamentoTime), po.portamento_time);
	_proxy_portamento_time->config()->curve = 1.0;
	_proxy_portamento_time->config()->user_limit_max = 0.5f * Params::Oscillator::PortamentoTimeDenominator;
	_proxy_portamento_time->apply_config();
	_proxy_phase = new ControllerProxy (_port_phase, &_oscillator_params.phase, 0, HARUHI_MIKURU_MINMAX (Params::Oscillator::Phase), po.phase);
	_proxy_noise_level = new ControllerProxy (_port_noise_level, &_oscillator_params.noise_level, 0, HARUHI_MIKURU_MINMAX (Params::Oscillator::NoiseLevel), po.noise_level);

	_control_volume = new Knob (_panel, _proxy_volume, "Volume", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Oscillator::Volume, 100), 2);
	_control_volume->set_unit_bay (_mikuru->unit_bay());
	_control_panorama = new Knob (_panel, _proxy_panorama, "Panorama", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::Panorama, 100), 2);
	_control_panorama->set_unit_bay (_mikuru->unit_bay());
	_control_detune = new Knob (_panel, _proxy_detune, "Detune", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::Detune, 100), 2);
	_control_detune->set_unit_bay (_mikuru->unit_bay());
	_control_pitchbend = new Knob (_panel, _proxy_pitchbend, "Pitch", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::Pitchbend, 100), 2);
	_control_pitchbend->set_unit_bay (_mikuru->unit_bay());
	_control_unison_index = new Knob (_panel, _proxy_unison_index, "Unison", HARUHI_MIKURU_PARAMS_FOR_KNOB (Params::Voice::UnisonIndex), 1, 0);
	_control_unison_index->set_unit_bay (_mikuru->unit_bay());
	_control_unison_spread = new Knob (_panel, _proxy_unison_spread, "U.spread", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::UnisonSpread, 100), 2);
	_control_unison_spread->set_unit_bay (_mikuru->unit_bay());
	_control_unison_init = new Knob (_panel, _proxy_unison_init, "U.init.φ", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::UnisonInit, 200), 2);
	_control_unison_init->set_unit_bay (_mikuru->unit_bay());
	_control_unison_noise = new Knob (_panel, _proxy_unison_noise, "U.noise", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::UnisonNoise, 100), 2);
	_control_unison_noise->set_unit_bay (_mikuru->unit_bay());
	_control_velocity_sens = new Knob (_panel, _proxy_velocity_sens, "Vel.sens.", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Voice::VelocitySens, 100), 2);
	_control_velocity_sens->set_unit_bay (_mikuru->unit_bay());
	_control_portamento_time = new Knob (_panel, _proxy_portamento_time, "Glide", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Oscillator::PortamentoTime, 100), 2);
	_control_portamento_time->set_unit_bay (_mikuru->unit_bay());
	_control_phase = new Knob (_panel, _proxy_phase, "Phase", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Oscillator::Phase, 200), 2);
	_control_phase->set_unit_bay (_mikuru->unit_bay());
	_control_noise_level = new Knob (_panel, _proxy_noise_level, "Noise lvl", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Oscillator::NoiseLevel, 200), 2);
	_control_noise_level->set_unit_bay (_mikuru->unit_bay());

	QObject::connect (_control_panorama, SIGNAL (changed (int)), this, SLOT (update_voice_panorama()));
	QObject::connect (_control_detune, SIGNAL (changed (int)), this, SLOT (update_voice_detune()));
	QObject::connect (_control_pitchbend, SIGNAL (changed (int)), this, SLOT (update_voice_pitchbend()));
	QObject::connect (_control_unison_index, SIGNAL (changed (int)), this, SLOT (update_voice_unison_index()));
	QObject::connect (_control_unison_spread, SIGNAL (changed (int)), this, SLOT (update_voice_unison_spread()));
	QObject::connect (_control_unison_init, SIGNAL (changed (int)), this, SLOT (update_voice_unison_init()));
	QObject::connect (_control_unison_noise, SIGNAL (changed (int)), this, SLOT (update_voice_unison_noise()));
	QObject::connect (_control_velocity_sens, SIGNAL (changed (int)), this, SLOT (update_voice_velocity_sens()));

	QToolTip::add (_control_unison_index, "Number of voices playing in unison");
	QToolTip::add (_control_unison_spread, "Unison frequencies spread");
	QToolTip::add (_control_unison_init, "Unison initial phases spread");
	QToolTip::add (_control_unison_noise, "Unison noise");
	QToolTip::add (_control_velocity_sens, "Velocity sensitivity (-1 for reverse, 0 for none)");

	// EventDispatchers for polyphonic controls:

	VoiceManager* vm = _part->voice_manager();
	_evdisp_amplitude = new EventDispatcher (_port_amplitude, Params::Voice::AmplitudeMin, Params::Voice::AmplitudeMax, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::amplitude));
	_evdisp_frequency = new EventDispatcher (_port_frequency, Params::Voice::FrequencyMin, Params::Voice::FrequencyMax, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::frequency));
	_evdisp_panorama = new EventDispatcher (_port_panorama, _control_panorama, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::panorama));
	_evdisp_detune = new EventDispatcher (_port_detune, _control_detune, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::detune));
	_evdisp_pitchbend = new EventDispatcher (_port_pitchbend, _control_pitchbend, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::pitchbend));
	_evdisp_velocity_sens = new EventDispatcher (_port_velocity_sens, _control_velocity_sens, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::velocity_sens));
	_evdisp_unison_index = new EventDispatcher (_port_unison_index, _control_unison_index, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::unison_index));
	_evdisp_unison_spread = new EventDispatcher (_port_unison_spread, _control_unison_spread, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::unison_spread));
	_evdisp_unison_init = new EventDispatcher (_port_unison_init, _control_unison_init, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::unison_init));
	_evdisp_unison_noise = new EventDispatcher (_port_unison_noise, _control_unison_noise, new EventDispatcher::VoiceParamReceiver (vm, &Params::Voice::unison_noise));

	// Grids:
	Q3GroupBox* grid1 = new Q3GroupBox (2, Qt::Horizontal, "", _panel);
	Q3GroupBox* grid2 = new Q3GroupBox (2, Qt::Horizontal, "", _panel);
	Q3GroupBox* grid4 = new Q3GroupBox (2, Qt::Horizontal, "", _panel);

	grid1->setInsideMargin (3 * Config::margin);
	grid2->setInsideMargin (3 * Config::margin);
	grid4->setInsideMargin (3 * Config::margin);

	// Monophonic:
	_monophonic = new QCheckBox ("Monophonic", grid1);
	_monophonic->setChecked (po.monophonic);
	QObject::connect (_monophonic, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	// Monophonic retrigger:
	_monophonic_retrigger = new QCheckBox ("Monophonic retrigger", grid1);
	_monophonic_retrigger->setChecked (po.monophonic_retrigger);
	QObject::connect (_monophonic_retrigger, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	// Key priority:
	new QLabel ("Monophonic key priority:", grid1);
	_monophonic_key_priority = new QComboBox (grid1);
	_monophonic_key_priority->insertItem ("Last pressed", Params::Oscillator::LastPressed);
	_monophonic_key_priority->insertItem ("First pressed", Params::Oscillator::FirstPressed);
	_monophonic_key_priority->insertItem ("Lowest", Params::Oscillator::LowestPressed);
	_monophonic_key_priority->insertItem ("Highest", Params::Oscillator::HighestPressed);
	_monophonic_key_priority->setCurrentItem (po.monophonic_key_priority);
	QObject::connect (_monophonic_key_priority, SIGNAL (activated (int)), this, SLOT (update_params()));

	// Transposition:
	new QLabel ("Transposition:", grid1);
	_transposition_semitones = new QSpinBox (-60, 60, 1, grid1);
	_transposition_semitones->setValue (po.transposition_semitones);
	QObject::connect (_transposition_semitones, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	// Glide:
	_const_portamento_time = new QCheckBox ("Const. portamento", grid1);
	_const_portamento_time->setChecked (po.const_portamento_time);
	QObject::connect (_const_portamento_time, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	// Pitchbend down/up:
	new QLabel ("Pitchbend down/up range:", grid2);
	QWidget* pitchbends_panel = new QWidget (grid2);
	QHBoxLayout* pitchbends_layout = new QHBoxLayout (pitchbends_panel, 0, Config::spacing);

	_pitchbend_down_semitones = new QSpinBox (0, 60, 1, pitchbends_panel);
	_pitchbend_down_semitones->setValue (po.pitchbend_down_semitones);
	QObject::connect (_pitchbend_down_semitones, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	_pitchbend_up_semitones = new QSpinBox (0, 60, 1, pitchbends_panel);
	_pitchbend_up_semitones->setValue (po.pitchbend_up_semitones);
	QObject::connect (_pitchbend_up_semitones, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	pitchbends_layout->addWidget (_pitchbend_down_semitones);
	pitchbends_layout->addWidget (_pitchbend_up_semitones);

	// Pitchbend enabled:
	_pitchbend_enabled = new QCheckBox ("Pitchbend enabled", grid2);
	_pitchbend_enabled->setChecked (po.pitchbend_enabled);
	QObject::connect (_pitchbend_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	// Pitchbend released:
	_pitchbend_released = new QCheckBox ("Bend released", grid2);
	_pitchbend_released->setChecked (po.pitchbend_released);
	QObject::connect (_pitchbend_released, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QToolTip::add (_pitchbend_released, "Alter pitch of already released voices");

	// Frequency modulation range:
	new QLabel ("Frequency mod. range:", grid2);
	_frequency_modulation_range = new QSpinBox (1, 60, 1, grid2);
	_frequency_modulation_range->setValue (po.frequency_mod_range);
	QObject::connect (_frequency_modulation_range, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	// Amplitude modulation smoothing:
	new QLabel ("Amp. mod. smoothing:", grid4);
	_amplitude_modulation_smoothing = new QSpinBox (0, 500, 5, grid4);
	_amplitude_modulation_smoothing->setSuffix (" ms");
	_amplitude_modulation_smoothing->setSpecialValueText ("Off");
	_amplitude_modulation_smoothing->setMinimumWidth (65);
	_amplitude_modulation_smoothing->setValue (po.amplitude_smoothing);
	QObject::connect (_amplitude_modulation_smoothing, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	// Frequency modulation smoothing:
	new QLabel ("Freq. mod. smoothing:", grid4);
	_frequency_modulation_smoothing = new QSpinBox (0, 500, 5, grid4);
	_frequency_modulation_smoothing->setSuffix (" ms");
	_frequency_modulation_smoothing->setSpecialValueText ("Off");
	_frequency_modulation_smoothing->setMinimumWidth (65);
	_frequency_modulation_smoothing->setValue (po.frequency_smoothing);
	QObject::connect (_frequency_modulation_smoothing, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	// Wave enabled:
	_wave_enabled = new QCheckBox ("Wave enabled", grid4);
	_wave_enabled->setChecked (po.wave_enabled);
	QObject::connect (_wave_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	// Noise enabled:
	_noise_enabled = new QCheckBox ("Noise enabled", grid4);
	_noise_enabled->setChecked (po.noise_enabled);
	QObject::connect (_noise_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	// Layouts:

	QVBoxLayout* layout = new QVBoxLayout (this, Config::margin, Config::spacing);
	layout->addWidget (_panel);

	QVBoxLayout* panel_layout = new QVBoxLayout (_panel, 0, Config::spacing);

	QHBoxLayout* layout2 = new QHBoxLayout (panel_layout, Config::spacing);
	QVBoxLayout* controls_layout = new QVBoxLayout (layout2, Config::spacing);

	QHBoxLayout* volumes_layout = new QHBoxLayout (controls_layout, Config::spacing);
	volumes_layout->addWidget (_control_volume);
	volumes_layout->addWidget (_control_panorama);
	volumes_layout->addWidget (_control_detune);
	volumes_layout->addWidget (_control_pitchbend);

	QHBoxLayout* unisons_layout = new QHBoxLayout (controls_layout, Config::spacing);
	unisons_layout->addWidget (_control_unison_index);
	unisons_layout->addWidget (_control_unison_spread);
	unisons_layout->addWidget (_control_unison_init);
	unisons_layout->addWidget (_control_unison_noise);

	QHBoxLayout* third_layout = new QHBoxLayout (controls_layout, Config::spacing);
	QVBoxLayout* third_v_layout = new QVBoxLayout (third_layout, Config::spacing);
	QHBoxLayout* third_h_layout = new QHBoxLayout (third_v_layout, Config::spacing);

	third_h_layout->addWidget (_control_velocity_sens);
	third_h_layout->addWidget (_control_portamento_time);
	third_h_layout->addWidget (_control_phase);
	third_h_layout->addWidget (_control_noise_level);
	third_v_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	controls_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	QVBoxLayout* widgets_layout = new QVBoxLayout (layout2, Config::spacing);
	widgets_layout->addWidget (grid1);
	widgets_layout->addWidget (grid2);
	widgets_layout->addWidget (grid4);

	update_widgets();
}


Oscillator::~Oscillator()
{
	// Delete knobs before ControllerProxies:
	delete _control_volume;
	delete _control_panorama;
	delete _control_detune;
	delete _control_pitchbend;
	delete _control_velocity_sens;
	delete _control_unison_index;
	delete _control_unison_spread;
	delete _control_unison_init;
	delete _control_unison_noise;
	delete _control_portamento_time;
	delete _control_phase;
	delete _control_noise_level;

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

	delete _proxy_volume;
	delete _proxy_portamento_time;
	delete _proxy_phase;
	delete _proxy_panorama;
	delete _proxy_detune;
	delete _proxy_pitchbend;
	delete _proxy_velocity_sens;
	delete _proxy_unison_index;
	delete _proxy_unison_spread;
	delete _proxy_unison_init;
	delete _proxy_unison_noise;
	delete _proxy_noise_level;

	_mikuru->graph()->lock();
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
	_mikuru->graph()->unlock();
}


void
Oscillator::process_events()
{
	// Oscillator:
	_proxy_volume->process_events();
	_proxy_portamento_time->process_events();
	_proxy_phase->process_events();
	_proxy_noise_level->process_events();
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


void
Oscillator::load_oscillator_params()
{
	_loading_params = true;

	Params::Oscillator po = _oscillator_params;

	// Knobs:
	_proxy_volume->set_value (po.volume);
	_proxy_portamento_time->set_value (po.portamento_time);
	_proxy_phase->set_value (po.phase);
	// Other:
	_wave_enabled->setChecked (po.wave_enabled);
	_noise_enabled->setChecked (po.noise_enabled);
	_frequency_modulation_range->setValue (po.frequency_mod_range);
	_pitchbend_enabled->setChecked (po.pitchbend_enabled);
	_pitchbend_released->setChecked (po.pitchbend_released);
	_pitchbend_up_semitones->setValue (po.pitchbend_up_semitones);
	_pitchbend_down_semitones->setValue (po.pitchbend_down_semitones);
	_transposition_semitones->setValue (po.transposition_semitones);
	_monophonic->setChecked (po.monophonic);
	_monophonic_retrigger->setChecked (po.monophonic_retrigger);
	_monophonic_key_priority->setCurrentItem (po.monophonic_key_priority);
	_const_portamento_time->setChecked (po.const_portamento_time);
	// Pitchbend/panorama smoothing are set by Knobs.
	_amplitude_modulation_smoothing->setValue (po.amplitude_smoothing);
	_frequency_modulation_smoothing->setValue (po.frequency_smoothing);

	_loading_params = false;
	update_widgets();
}


void
Oscillator::load_voice_params()
{
	if (_loading_params)
		return;
	_loading_params = true;

	Params::Voice pv = _voice_params;

	// Knobs:
	_proxy_panorama->set_value (pv.panorama);
	_proxy_detune->set_value (pv.detune);
	_proxy_pitchbend->set_value (pv.pitchbend);
	_proxy_velocity_sens->set_value (pv.velocity_sens);
	_proxy_unison_index->set_value (pv.unison_index);
	_proxy_unison_spread->set_value (pv.unison_spread);
	_proxy_unison_init->set_value (pv.unison_init);
	_proxy_unison_noise->set_value (pv.unison_noise);

	_loading_params = false;
	update_widgets();
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
	po.pitchbend_down_semitones = _pitchbend_down_semitones->value();
	po.transposition_semitones = _transposition_semitones->value();
	po.monophonic = _monophonic->isChecked();
	po.monophonic_retrigger = _monophonic_retrigger->isChecked();
	po.monophonic_key_priority = _monophonic_key_priority->currentItem();
	po.const_portamento_time = _const_portamento_time->isChecked();
	po.amplitude_smoothing = _amplitude_modulation_smoothing->value();
	po.frequency_smoothing = _frequency_modulation_smoothing->value();
	po.panorama_smoothing = _oscillator_params.panorama_smoothing;
	po.pitchbend_smoothing = _oscillator_params.pitchbend_smoothing;

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
}

} // namespace MikuruPrivate

