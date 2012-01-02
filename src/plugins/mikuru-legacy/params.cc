/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <algorithm>

// Haruhi:
#include <haruhi/utility/numeric.h>
#include <haruhi/dsp/modulated_wave.h>

// Local:
#include "params.h"
#include "rbj_impulse_response.h"


#define HARUHI_MIKURU_CONSTRUCT(var, name) \
	var (name##Min, name##Max, name##Default, name##Denominator, #var)

#define HARUHI_MIKURU_COPY(var) \
	var = other.var;

#define HARUHI_MIKURU_SANITIZE(var) \
	var.sanitize();


namespace MikuruPrivate {

Params::General::General():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (volume, Volume),
	HARUHI_MIKURU_CONSTRUCT (detune, Detune),
	HARUHI_MIKURU_CONSTRUCT (panorama, Panorama),
	HARUHI_MIKURU_CONSTRUCT (stereo_width, StereoWidth),
	HARUHI_MIKURU_CONSTRUCT (input_volume, InputVolume),
	// Non-controller:
	polyphony (0, 512, 32, "polyphony"),
	enable_audio_input (0, 1, 0, "enable_audio_input")
{
}


void
Params::General::set_controller_params (General& other)
{
	HARUHI_MIKURU_COPY (volume)
	HARUHI_MIKURU_COPY (detune)
	HARUHI_MIKURU_COPY (panorama)
	HARUHI_MIKURU_COPY (stereo_width)
	HARUHI_MIKURU_COPY (input_volume)
}


void
Params::General::set_non_controller_params (General& other)
{
	HARUHI_MIKURU_COPY (polyphony)
	HARUHI_MIKURU_COPY (enable_audio_input)
}


void
Params::General::sanitize()
{
	HARUHI_MIKURU_SANITIZE (volume)
	HARUHI_MIKURU_SANITIZE (detune)
	HARUHI_MIKURU_SANITIZE (panorama)
	HARUHI_MIKURU_SANITIZE (stereo_width)
	HARUHI_MIKURU_SANITIZE (input_volume)
	HARUHI_MIKURU_SANITIZE (polyphony)
	HARUHI_MIKURU_SANITIZE (enable_audio_input)
}


Params::Filter::Filter():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (frequency, Frequency),
	HARUHI_MIKURU_CONSTRUCT (resonance, Resonance),
	HARUHI_MIKURU_CONSTRUCT (gain, Gain),
	HARUHI_MIKURU_CONSTRUCT (attenuation, Attenuation),
	// Non-controller:
	enabled (0, 1, 0, "enabled"),
	type (0, 7, RBJImpulseResponse::LowPass, "type"),
	stages (1, 5, 1, "stages"),
	limiter_enabled (0, 1, 1, "limiter_enabled")
{
}


void
Params::Filter::set_controller_params (Filter& other)
{
	HARUHI_MIKURU_COPY (frequency)
	HARUHI_MIKURU_COPY (resonance)
	HARUHI_MIKURU_COPY (gain)
	HARUHI_MIKURU_COPY (attenuation)
}


void
Params::Filter::set_non_controller_params (Filter& other)
{
	HARUHI_MIKURU_COPY (enabled)
	HARUHI_MIKURU_COPY (type)
	HARUHI_MIKURU_COPY (stages)
	HARUHI_MIKURU_COPY (limiter_enabled)
}


void
Params::Filter::sanitize()
{
	HARUHI_MIKURU_SANITIZE (frequency)
	HARUHI_MIKURU_SANITIZE (resonance)
	HARUHI_MIKURU_SANITIZE (gain)
	HARUHI_MIKURU_SANITIZE (attenuation)
	HARUHI_MIKURU_SANITIZE (enabled)
	HARUHI_MIKURU_SANITIZE (type)
	HARUHI_MIKURU_SANITIZE (stages)
	HARUHI_MIKURU_SANITIZE (limiter_enabled)
}


Params::CommonFilters::CommonFilters():
	// Non-controller:
	filter_configuration (0, 1, FilterConfigurationSerial, "filter_configuration"),
	route_audio_input (0, 1, 0, "route_audio_input")
{
}


void
Params::CommonFilters::set_controller_params (CommonFilters&)
{
}


void
Params::CommonFilters::set_non_controller_params (CommonFilters& other)
{
	HARUHI_MIKURU_COPY (filter_configuration)
	HARUHI_MIKURU_COPY (route_audio_input)
}


void
Params::CommonFilters::sanitize()
{
	HARUHI_MIKURU_SANITIZE (filter_configuration)
	HARUHI_MIKURU_SANITIZE (route_audio_input)
}


Params::Part::Part():
	// Non-controller:
	enabled (0, 1, 1, "enabled")
{
}


void
Params::Part::set_controller_params (Part&)
{
}


void
Params::Part::set_non_controller_params (Part& other)
{
	HARUHI_MIKURU_COPY (enabled)
}


void
Params::Part::sanitize()
{
	HARUHI_MIKURU_SANITIZE (enabled)
}


Params::PartFilters::PartFilters():
	// Non-controller:
	filter_configuration (0, 1, FilterConfigurationSerial, "filter_configuration")
{
}


void
Params::PartFilters::set_controller_params (PartFilters&)
{
}


void
Params::PartFilters::set_non_controller_params (PartFilters& other)
{
	HARUHI_MIKURU_COPY (filter_configuration)
}


void
Params::PartFilters::sanitize()
{
	HARUHI_MIKURU_SANITIZE (filter_configuration)
}


Params::Waveform::Waveform():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (wave_shape, WaveShape),
	HARUHI_MIKURU_CONSTRUCT (modulator_amplitude, ModulatorAmplitude),
	HARUHI_MIKURU_CONSTRUCT (modulator_index, ModulatorIndex),
	HARUHI_MIKURU_CONSTRUCT (modulator_shape, ModulatorShape),
	// Non-controller:
	wave_type (0, 8, 0, "wave_type"),
	modulator_type (0, 1, DSP::ModulatedWave::Ring, "modulator_type"),
	modulator_wave_type (0, 3, 0, "modulator_wave_type")
{
	for (int i = 0; i < HarmonicsNumber; ++i)
		harmonics[i] = Haruhi::Param<int> (HarmonicMin, HarmonicMax, HarmonicDefault, "harmonic");
	for (int i = 0; i < HarmonicsNumber; ++i)
		phases[i] = Haruhi::Param<int> (PhaseMin, PhaseMax, PhaseDefault, "phase");
}


void
Params::Waveform::set_controller_params (Waveform& other)
{
	HARUHI_MIKURU_COPY (wave_shape)
	HARUHI_MIKURU_COPY (modulator_amplitude)
	HARUHI_MIKURU_COPY (modulator_index)
	HARUHI_MIKURU_COPY (modulator_shape)
}


void
Params::Waveform::set_non_controller_params (Waveform& other)
{
	HARUHI_MIKURU_COPY (wave_type)
	HARUHI_MIKURU_COPY (modulator_type)
	HARUHI_MIKURU_COPY (modulator_wave_type)
	for (int i = 0; i < HarmonicsNumber; ++i)
		HARUHI_MIKURU_COPY (harmonics[i]);
	for (int i = 0; i < HarmonicsNumber; ++i)
		HARUHI_MIKURU_COPY (phases[i]);
}


void
Params::Waveform::sanitize()
{
	HARUHI_MIKURU_SANITIZE (wave_shape)
	HARUHI_MIKURU_SANITIZE (modulator_amplitude)
	HARUHI_MIKURU_SANITIZE (modulator_index)
	HARUHI_MIKURU_SANITIZE (modulator_shape)
	HARUHI_MIKURU_SANITIZE (wave_type)
	HARUHI_MIKURU_SANITIZE (modulator_type)
	HARUHI_MIKURU_SANITIZE (modulator_wave_type)
	for (int i = 0; i < HarmonicsNumber; ++i)
		HARUHI_MIKURU_SANITIZE (harmonics[i]);
	for (int i = 0; i < HarmonicsNumber; ++i)
		HARUHI_MIKURU_SANITIZE (phases[i]);
}


Params::Oscillator::Oscillator():
	// Contoller:
	HARUHI_MIKURU_CONSTRUCT (volume, Volume),
	HARUHI_MIKURU_CONSTRUCT (portamento_time, PortamentoTime),
	HARUHI_MIKURU_CONSTRUCT (phase, Phase),
	HARUHI_MIKURU_CONSTRUCT (noise_level, NoiseLevel),
	// Non-controller:
	wave_enabled (0, 1, 1, "wave_enabled"),
	noise_enabled (0, 1, 0, "noise_enabled"),
	frequency_mod_range (0, 60, 12, "frequency_mod_range"),
	pitchbend_enabled (0, 1, 1, "pitchbend_enabled"),
	pitchbend_released (0, 1, 0, "pitchbend_released"),
	pitchbend_up_semitones (0, 60, 2, "pitchbend_up_semitones"),
	pitchbend_down_semitones (0, 60, 2, "pitchbend_down_semitones"),
	transposition_semitones (-60, 60, 0, "transposition_semitones"),
	monophonic (0, 1, 0, "monophonic"),
	monophonic_retrigger (0, 1, 0, "monophonic_retrigger"),
	monophonic_key_priority (0, 3, LastPressed, "monophonic_key_priority"),
	const_portamento_time (0, 1, 1, "const_portamento_time"),
	unison_stereo (0, 1, 1, "unison_stereo"),
	pseudo_stereo (0, 1, 0, "pseudo_stereo")
{
}


void
Params::Oscillator::set_controller_params (Oscillator& other)
{
	HARUHI_MIKURU_COPY (volume)
	HARUHI_MIKURU_COPY (portamento_time)
	HARUHI_MIKURU_COPY (phase)
	HARUHI_MIKURU_COPY (noise_level)
}


void
Params::Oscillator::set_non_controller_params (Oscillator& other)
{
	HARUHI_MIKURU_COPY (wave_enabled)
	HARUHI_MIKURU_COPY (noise_enabled)
	HARUHI_MIKURU_COPY (frequency_mod_range)
	HARUHI_MIKURU_COPY (pitchbend_enabled)
	HARUHI_MIKURU_COPY (pitchbend_released)
	HARUHI_MIKURU_COPY (pitchbend_up_semitones)
	HARUHI_MIKURU_COPY (pitchbend_down_semitones)
	HARUHI_MIKURU_COPY (transposition_semitones)
	HARUHI_MIKURU_COPY (monophonic)
	HARUHI_MIKURU_COPY (monophonic_retrigger)
	HARUHI_MIKURU_COPY (monophonic_key_priority)
	HARUHI_MIKURU_COPY (const_portamento_time)
	HARUHI_MIKURU_COPY (unison_stereo)
	HARUHI_MIKURU_COPY (pseudo_stereo)
}


void
Params::Oscillator::sanitize()
{
	HARUHI_MIKURU_SANITIZE (volume)
	HARUHI_MIKURU_SANITIZE (portamento_time)
	HARUHI_MIKURU_SANITIZE (phase)
	HARUHI_MIKURU_SANITIZE (noise_level)
	HARUHI_MIKURU_SANITIZE (wave_enabled)
	HARUHI_MIKURU_SANITIZE (noise_enabled)
	HARUHI_MIKURU_SANITIZE (frequency_mod_range)
	HARUHI_MIKURU_SANITIZE (pitchbend_enabled)
	HARUHI_MIKURU_SANITIZE (pitchbend_released)
	HARUHI_MIKURU_SANITIZE (pitchbend_up_semitones)
	HARUHI_MIKURU_SANITIZE (pitchbend_down_semitones)
	HARUHI_MIKURU_SANITIZE (transposition_semitones)
	HARUHI_MIKURU_SANITIZE (monophonic)
	HARUHI_MIKURU_SANITIZE (monophonic_retrigger)
	HARUHI_MIKURU_SANITIZE (monophonic_key_priority)
	HARUHI_MIKURU_SANITIZE (const_portamento_time)
	HARUHI_MIKURU_SANITIZE (unison_stereo)
	HARUHI_MIKURU_SANITIZE (pseudo_stereo)
}


Params::Voice::Voice():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (adsr, Adsr),
	HARUHI_MIKURU_CONSTRUCT (amplitude, Amplitude),
	HARUHI_MIKURU_CONSTRUCT (frequency, Frequency),
	HARUHI_MIKURU_CONSTRUCT (panorama, Panorama),
	HARUHI_MIKURU_CONSTRUCT (detune, Detune),
	HARUHI_MIKURU_CONSTRUCT (pitchbend, Pitchbend),
	HARUHI_MIKURU_CONSTRUCT (velocity_sens, VelocitySens),
	HARUHI_MIKURU_CONSTRUCT (unison_index, UnisonIndex),
	HARUHI_MIKURU_CONSTRUCT (unison_spread, UnisonSpread),
	HARUHI_MIKURU_CONSTRUCT (unison_init, UnisonInit),
	HARUHI_MIKURU_CONSTRUCT (unison_noise, UnisonNoise)
{
}


void
Params::Voice::set_controller_params (Voice& other)
{
	HARUHI_MIKURU_COPY (panorama)
	HARUHI_MIKURU_COPY (detune)
	HARUHI_MIKURU_COPY (pitchbend)
	HARUHI_MIKURU_COPY (velocity_sens)
	HARUHI_MIKURU_COPY (unison_index)
	HARUHI_MIKURU_COPY (unison_spread)
	HARUHI_MIKURU_COPY (unison_init)
	HARUHI_MIKURU_COPY (unison_noise)
	HARUHI_MIKURU_COPY (adsr)
	HARUHI_MIKURU_COPY (amplitude)
	HARUHI_MIKURU_COPY (frequency)
}


void
Params::Voice::set_non_controller_params (Voice& other)
{
}


void
Params::Voice::sanitize()
{
	HARUHI_MIKURU_SANITIZE (panorama)
	HARUHI_MIKURU_SANITIZE (detune)
	HARUHI_MIKURU_SANITIZE (pitchbend)
	HARUHI_MIKURU_SANITIZE (velocity_sens)
	HARUHI_MIKURU_SANITIZE (unison_index)
	HARUHI_MIKURU_SANITIZE (unison_spread)
	HARUHI_MIKURU_SANITIZE (unison_init)
	HARUHI_MIKURU_SANITIZE (unison_noise)
	HARUHI_MIKURU_SANITIZE (adsr)
	HARUHI_MIKURU_SANITIZE (amplitude)
	HARUHI_MIKURU_SANITIZE (frequency)
}


Params::ADSR::ADSR():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (delay, Delay),
	HARUHI_MIKURU_CONSTRUCT (attack, Attack),
	HARUHI_MIKURU_CONSTRUCT (attack_hold, AttackHold),
	HARUHI_MIKURU_CONSTRUCT (decay, Decay),
	HARUHI_MIKURU_CONSTRUCT (sustain, Sustain),
	HARUHI_MIKURU_CONSTRUCT (sustain_hold, SustainHold),
	HARUHI_MIKURU_CONSTRUCT (release, Release),
	// Non-controller:
	enabled (0, 1, 1, "enabled"),
	direct_adsr (0, 1, 1, "direct_adsr"),
	forced_release (0, 1, 0, "forced_release"),
	sustain_enabled (0, 1, 1, "sustain_enabled"),
	function (0, 4, Linear, "function"),
	mode (0, 1, Polyphonic, "mode")
{
}


void
Params::ADSR::set_controller_params (ADSR& other)
{
	HARUHI_MIKURU_COPY (delay)
	HARUHI_MIKURU_COPY (attack)
	HARUHI_MIKURU_COPY (attack_hold)
	HARUHI_MIKURU_COPY (decay)
	HARUHI_MIKURU_COPY (sustain)
	HARUHI_MIKURU_COPY (sustain_hold)
	HARUHI_MIKURU_COPY (release)
}


void
Params::ADSR::set_non_controller_params (ADSR& other)
{
	HARUHI_MIKURU_COPY (enabled)
	HARUHI_MIKURU_COPY (direct_adsr)
	HARUHI_MIKURU_COPY (forced_release)
	HARUHI_MIKURU_COPY (sustain_enabled)
	HARUHI_MIKURU_COPY (function)
	HARUHI_MIKURU_COPY (mode)
}


void
Params::ADSR::sanitize()
{
	HARUHI_MIKURU_SANITIZE (delay)
	HARUHI_MIKURU_SANITIZE (attack)
	HARUHI_MIKURU_SANITIZE (attack_hold)
	HARUHI_MIKURU_SANITIZE (decay)
	HARUHI_MIKURU_SANITIZE (sustain)
	HARUHI_MIKURU_SANITIZE (sustain_hold)
	HARUHI_MIKURU_SANITIZE (release)
	HARUHI_MIKURU_SANITIZE (enabled)
	HARUHI_MIKURU_SANITIZE (direct_adsr)
	HARUHI_MIKURU_SANITIZE (forced_release)
	HARUHI_MIKURU_SANITIZE (sustain_enabled)
	HARUHI_MIKURU_SANITIZE (function)
	HARUHI_MIKURU_SANITIZE (mode)
}


Params::LFO::LFO():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (delay, Delay),
	HARUHI_MIKURU_CONSTRUCT (fade_in, FadeIn),
	HARUHI_MIKURU_CONSTRUCT (frequency, Frequency),
	HARUHI_MIKURU_CONSTRUCT (level, Level),
	HARUHI_MIKURU_CONSTRUCT (depth, Depth),
	HARUHI_MIKURU_CONSTRUCT (phase, Phase),
	HARUHI_MIKURU_CONSTRUCT (wave_shape, WaveShape),
	HARUHI_MIKURU_CONSTRUCT (fade_out, FadeOut),
	// Non-controller:
	enabled (0, 1, 1, "enabled"),
	wave_type (0, 6, Sine, "wave_type"),
	wave_invert (0, 1, 0, "wave_invert"),
	function (0, 4, Linear, "function"),
	mode (0, 2, Polyphonic, "mode"),
	tempo_sync (0, 1, 0, "tempo_sync"),
	tempo_numerator (1, 32, 1, "tempo_numerator"),
	tempo_denominator (1, 32, 1, "tempo_denominator"),
	random_start_phase (0, 1, 0, "random_start_phase"),
	fade_out_enabled (0, 1, 0, "fade_out_enabled")
{
}


void
Params::LFO::set_controller_params (LFO& other)
{
	HARUHI_MIKURU_COPY (delay)
	HARUHI_MIKURU_COPY (fade_in)
	HARUHI_MIKURU_COPY (frequency)
	HARUHI_MIKURU_COPY (level)
	HARUHI_MIKURU_COPY (depth)
	HARUHI_MIKURU_COPY (phase)
	HARUHI_MIKURU_COPY (wave_shape)
	HARUHI_MIKURU_COPY (fade_out)
}


void
Params::LFO::set_non_controller_params (LFO& other)
{
	HARUHI_MIKURU_COPY (enabled)
	HARUHI_MIKURU_COPY (wave_type)
	HARUHI_MIKURU_COPY (wave_invert)
	HARUHI_MIKURU_COPY (function)
	HARUHI_MIKURU_COPY (mode)
	HARUHI_MIKURU_COPY (tempo_sync)
	HARUHI_MIKURU_COPY (tempo_numerator)
	HARUHI_MIKURU_COPY (tempo_denominator)
	HARUHI_MIKURU_COPY (random_start_phase)
	HARUHI_MIKURU_COPY (fade_out_enabled)
}


void
Params::LFO::sanitize()
{
	HARUHI_MIKURU_SANITIZE (delay)
	HARUHI_MIKURU_SANITIZE (fade_in)
	HARUHI_MIKURU_SANITIZE (frequency)
	HARUHI_MIKURU_SANITIZE (level)
	HARUHI_MIKURU_SANITIZE (depth)
	HARUHI_MIKURU_SANITIZE (phase)
	HARUHI_MIKURU_SANITIZE (wave_shape)
	HARUHI_MIKURU_SANITIZE (fade_out)
	HARUHI_MIKURU_SANITIZE (enabled)
	HARUHI_MIKURU_SANITIZE (wave_type)
	HARUHI_MIKURU_SANITIZE (wave_invert)
	HARUHI_MIKURU_SANITIZE (function)
	HARUHI_MIKURU_SANITIZE (mode)
	HARUHI_MIKURU_SANITIZE (tempo_sync)
	HARUHI_MIKURU_SANITIZE (tempo_numerator)
	HARUHI_MIKURU_SANITIZE (tempo_denominator)
	HARUHI_MIKURU_SANITIZE (random_start_phase)
	HARUHI_MIKURU_SANITIZE (fade_out_enabled)
}


Params::EG::EG():
	// Controller:
	// Non-controller:
	enabled (0, 1, 1, "enabled"),
	segments (2, MaxPoints - 1, 2, "segments"),
	sustain_point (1, MaxPoints - 1, 0, "sustain_point")
{
	for (int i = 0; i < MaxPoints; ++i)
	{
		values[i] = Haruhi::Param<unsigned int> (PointValueMin, PointValueMax, PointValueDefault, "value");
		durations[i] = Haruhi::Param<unsigned int> (SegmentDurationMin, SegmentDurationMax, SegmentDurationDefault, "duration");
	}
}


void
Params::EG::set_controller_params (EG& other)
{
}


void
Params::EG::set_non_controller_params (EG& other)
{
	HARUHI_MIKURU_COPY (enabled)
	HARUHI_MIKURU_COPY (segments)
	HARUHI_MIKURU_COPY (sustain_point)
	// Copy tables:
	for (int i = 0; i < MaxPoints; ++i)
	{
		HARUHI_MIKURU_COPY (values[i])
		HARUHI_MIKURU_COPY (durations[i])
	}
}


void
Params::EG::sanitize()
{
	HARUHI_MIKURU_SANITIZE (enabled)
	HARUHI_MIKURU_SANITIZE (segments)
	HARUHI_MIKURU_SANITIZE (sustain_point)

	int p = segments.get();
	if (p < 2)
	{
		p = 2;
		segments.set (p);
	}

	int s = sustain_point.get();
	if (s >= p)
	{
		s = p;
		sustain_point = s;
	}

	for (int i = 0; i < MaxPoints; ++i)
	{
		HARUHI_MIKURU_SANITIZE (values[i])
		HARUHI_MIKURU_SANITIZE (durations[i])
	}
}


Params::Effect::Effect():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (wet, Wet),
	HARUHI_MIKURU_CONSTRUCT (panorama, Panorama),
	// Non-controller:
	enabled (0, 1, 1, "enabled")
{
}


void
Params::Effect::set_controller_params (Effect& other)
{
	HARUHI_MIKURU_COPY (wet)
	HARUHI_MIKURU_COPY (panorama)
}


void
Params::Effect::set_non_controller_params (Effect& other)
{
	HARUHI_MIKURU_COPY (enabled)
}


void
Params::Effect::sanitize()
{
	HARUHI_MIKURU_SANITIZE (enabled)
	HARUHI_MIKURU_SANITIZE (wet)
	HARUHI_MIKURU_SANITIZE (panorama)
}


Params::Waveshaper::Waveshaper():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (gain, Gain),
	HARUHI_MIKURU_CONSTRUCT (parameter, Parameter),
	// Non-controller:
	type (0, 3, 0, "type")
{
}


void
Params::Waveshaper::set_controller_params (Waveshaper& other)
{
	Effect::set_controller_params (other);

	HARUHI_MIKURU_COPY (gain)
	HARUHI_MIKURU_COPY (parameter)
}


void
Params::Waveshaper::set_non_controller_params (Waveshaper& other)
{
	Effect::set_non_controller_params (other);

	HARUHI_MIKURU_COPY (type)
}


void
Params::Waveshaper::sanitize()
{
	Effect::sanitize();

	HARUHI_MIKURU_SANITIZE (gain)
	HARUHI_MIKURU_SANITIZE (parameter)
	HARUHI_MIKURU_SANITIZE (type)
}


Params::Reverb::Reverb():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (room_size, RoomSize),
	HARUHI_MIKURU_CONSTRUCT (width, Width),
	HARUHI_MIKURU_CONSTRUCT (damp, Damp),
	// Non-controller:
	mode (0, 1, 0, "mode")
{
}


void
Params::Reverb::set_controller_params (Reverb& other)
{
	Effect::set_controller_params (other);

	HARUHI_MIKURU_COPY (room_size)
	HARUHI_MIKURU_COPY (width)
	HARUHI_MIKURU_COPY (damp)
}


void
Params::Reverb::set_non_controller_params (Reverb& other)
{
	Effect::set_non_controller_params (other);

	HARUHI_MIKURU_COPY (mode)
}


void
Params::Reverb::sanitize()
{
	Effect::sanitize();

	HARUHI_MIKURU_SANITIZE (room_size)
	HARUHI_MIKURU_SANITIZE (width)
	HARUHI_MIKURU_SANITIZE (damp)
	HARUHI_MIKURU_SANITIZE (mode)
}


Params::Delay::Delay():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (feedback_l, Feedback),
	HARUHI_MIKURU_CONSTRUCT (feedback_r, Feedback),
	HARUHI_MIKURU_CONSTRUCT (cross_feedback_l, CrossFeedback),
	HARUHI_MIKURU_CONSTRUCT (cross_feedback_r, CrossFeedback),
	HARUHI_MIKURU_CONSTRUCT (level_l, Level),
	HARUHI_MIKURU_CONSTRUCT (level_r, Level),
	// Non-controller:
	tempo (20000, 400000, 120000, "tempo"),
	enabled_l (0, 1, 1, "enabled_l"),
	enabled_r (0, 1, 1, "enabled_r"),
	note_length_l (1, 32, 4, "note_length_l"),
	note_length_r (1, 32, 4, "note_length_r"),
	note_multiplicator_l (1, 32, 1, "note_multiplicator_l"),
	note_multiplicator_r (1, 32, 1, "note_multiplicator_r"),
	note_adjust_l (-2500, +2500, 0, "note_adjust_l"),
	note_adjust_r (-2500, +2500, 0, "note_adjust_r")
{
}


void
Params::Delay::set_controller_params (Delay& other)
{
	Effect::set_controller_params (other);

	HARUHI_MIKURU_COPY (feedback_l)
	HARUHI_MIKURU_COPY (feedback_r)
	HARUHI_MIKURU_COPY (cross_feedback_l)
	HARUHI_MIKURU_COPY (cross_feedback_r)
	HARUHI_MIKURU_COPY (level_l)
	HARUHI_MIKURU_COPY (level_r)
}


void
Params::Delay::set_non_controller_params (Delay& other)
{
	Effect::set_non_controller_params (other);

	HARUHI_MIKURU_COPY (tempo)
	HARUHI_MIKURU_COPY (enabled_l)
	HARUHI_MIKURU_COPY (enabled_r)
	HARUHI_MIKURU_COPY (note_length_l)
	HARUHI_MIKURU_COPY (note_length_r)
	HARUHI_MIKURU_COPY (note_multiplicator_l)
	HARUHI_MIKURU_COPY (note_multiplicator_r)
	HARUHI_MIKURU_COPY (note_adjust_l)
	HARUHI_MIKURU_COPY (note_adjust_r)
}


void
Params::Delay::sanitize()
{
	Effect::sanitize();

	HARUHI_MIKURU_SANITIZE (feedback_l)
	HARUHI_MIKURU_SANITIZE (feedback_r)
	HARUHI_MIKURU_SANITIZE (cross_feedback_l)
	HARUHI_MIKURU_SANITIZE (cross_feedback_r)
	HARUHI_MIKURU_SANITIZE (level_l)
	HARUHI_MIKURU_SANITIZE (level_r)
	HARUHI_MIKURU_SANITIZE (tempo)
	HARUHI_MIKURU_SANITIZE (enabled_l)
	HARUHI_MIKURU_SANITIZE (enabled_r)
	HARUHI_MIKURU_SANITIZE (note_length_l)
	HARUHI_MIKURU_SANITIZE (note_length_r)
	HARUHI_MIKURU_SANITIZE (note_multiplicator_l)
	HARUHI_MIKURU_SANITIZE (note_multiplicator_r)
	HARUHI_MIKURU_SANITIZE (note_adjust_l)
	HARUHI_MIKURU_SANITIZE (note_adjust_r)
}

} // namespace MikuruPrivate

#undef HARUHI_MIKURU_COPY_ATOMIC
#undef HARUHI_MIKURU_COPY
#undef HARUHI_MIKURU_CONSTRUCT

