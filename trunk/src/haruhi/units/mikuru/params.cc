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
#include <algorithm>

// Haruhi:
#include <haruhi/utility/atomic.h>
#include <haruhi/dsp/modulated_wave.h>

// Local:
#include "params.h"
#include "rbj_impulse_response.h"


#define HARUHI_MIKURU_CONSTRUCT(var, name) \
	var (name##Min, name##Max, name##Denominator, name##Default)

#define HARUHI_MIKURU_COPY(name) \
	name = other.name;

#define HARUHI_MIKURU_COPY_ATOMIC(name) \
	name = atomic (other.name);


namespace MikuruPrivate {

Params::General::General():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (volume, Volume),
	HARUHI_MIKURU_CONSTRUCT (detune, Detune),
	HARUHI_MIKURU_CONSTRUCT (panorama, Panorama),
	HARUHI_MIKURU_CONSTRUCT (stereo_width, StereoWidth),
	HARUHI_MIKURU_CONSTRUCT (input_volume, InputVolume),
	// Non-controller:
	polyphony (32),
	enable_audio_input (0)
{
	panorama.set_smoothing (50);
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
	HARUHI_MIKURU_COPY_ATOMIC (polyphony)
	HARUHI_MIKURU_COPY_ATOMIC (enable_audio_input)
}


Params::Filter::Filter():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (frequency, Frequency),
	HARUHI_MIKURU_CONSTRUCT (resonance, Resonance),
	HARUHI_MIKURU_CONSTRUCT (gain, Gain),
	HARUHI_MIKURU_CONSTRUCT (attenuation, Attenuation),
	// Non-controller:
	enabled (0),
	type (RBJImpulseResponse::LowPass),
	passes (1),
	limiter_enabled (true)
{
	frequency.set_smoothing (50);
	resonance.set_smoothing (50);
	gain.set_smoothing (50);
	attenuation.set_smoothing (50);
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
	HARUHI_MIKURU_COPY_ATOMIC (enabled)
	HARUHI_MIKURU_COPY_ATOMIC (type)
	HARUHI_MIKURU_COPY_ATOMIC (passes)
	HARUHI_MIKURU_COPY_ATOMIC (limiter_enabled)
}


Params::CommonFilters::CommonFilters():
	// Non-controller:
	filter_configuration (FilterConfigurationSerial),
	route_audio_input (0)
{
}


void
Params::CommonFilters::set_controller_params (CommonFilters&)
{
}


void
Params::CommonFilters::set_non_controller_params (CommonFilters& other)
{
	HARUHI_MIKURU_COPY_ATOMIC (filter_configuration)
	HARUHI_MIKURU_COPY_ATOMIC (route_audio_input)
}


Params::Part::Part():
	// Non-controller:
	enabled (1)
{
}


void
Params::Part::set_controller_params (Part&)
{
}


void
Params::Part::set_non_controller_params (Part& other)
{
	HARUHI_MIKURU_COPY_ATOMIC (enabled)
}


Params::PartFilters::PartFilters():
	// Non-controller:
	filter_configuration (FilterConfigurationSerial)
{
}


void
Params::PartFilters::set_controller_params (PartFilters&)
{
}


void
Params::PartFilters::set_non_controller_params (PartFilters& other)
{
	HARUHI_MIKURU_COPY_ATOMIC (filter_configuration)
}


Params::Waveform::Waveform():
	// Controller:
	HARUHI_MIKURU_CONSTRUCT (wave_shape, WaveShape),
	HARUHI_MIKURU_CONSTRUCT (modulator_amplitude, ModulatorAmplitude),
	HARUHI_MIKURU_CONSTRUCT (modulator_index, ModulatorIndex),
	HARUHI_MIKURU_CONSTRUCT (modulator_shape, ModulatorShape),
	// Non-controller:
	wave_type (0),
	modulator_type (DSP::ModulatedWave::Ring),
	modulator_wave_type (0)
{
	for (int i = 0; i < HarmonicsNumber; ++i)
		harmonics[i] = HarmonicDefault;
	for (int i = 0; i < HarmonicsNumber; ++i)
		phases[i] = PhaseDefault;
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
	HARUHI_MIKURU_COPY_ATOMIC (wave_type)
	HARUHI_MIKURU_COPY_ATOMIC (modulator_type)
	HARUHI_MIKURU_COPY_ATOMIC (modulator_wave_type)
	for (int i = 0; i < HarmonicsNumber; ++i)
		harmonics[i] = atomic (other.harmonics[i]);
	for (int i = 0; i < HarmonicsNumber; ++i)
		phases[i] = atomic (other.phases[i]);
}


Params::Oscillator::Oscillator():
	// Contoller:
	HARUHI_MIKURU_CONSTRUCT (volume, Volume),
	HARUHI_MIKURU_CONSTRUCT (portamento_time, PortamentoTime),
	HARUHI_MIKURU_CONSTRUCT (phase, Phase),
	HARUHI_MIKURU_CONSTRUCT (noise_level, NoiseLevel),
	// Non-controller:
	wave_enabled (1),
	noise_enabled (0),
	frequency_mod_range (60),
	pitchbend_enabled (1),
	pitchbend_released (0),
	pitchbend_up_semitones (2),
	pitchbend_down_semitones (2),
	transposition_semitones (0),
	monophonic (0),
	monophonic_retrigger (0),
	monophonic_key_priority (LastPressed),
	const_portamento_time (1),
	amplitude_smoothing (50),
	frequency_smoothing (50),
	pitchbend_smoothing (50),
	panorama_smoothing (50)
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
	HARUHI_MIKURU_COPY_ATOMIC (wave_enabled)
	HARUHI_MIKURU_COPY_ATOMIC (noise_enabled)
	HARUHI_MIKURU_COPY_ATOMIC (frequency_mod_range)
	HARUHI_MIKURU_COPY_ATOMIC (pitchbend_enabled)
	HARUHI_MIKURU_COPY_ATOMIC (pitchbend_released)
	HARUHI_MIKURU_COPY_ATOMIC (pitchbend_up_semitones)
	HARUHI_MIKURU_COPY_ATOMIC (pitchbend_down_semitones)
	HARUHI_MIKURU_COPY_ATOMIC (transposition_semitones)
	HARUHI_MIKURU_COPY_ATOMIC (monophonic)
	HARUHI_MIKURU_COPY_ATOMIC (monophonic_retrigger)
	HARUHI_MIKURU_COPY_ATOMIC (monophonic_key_priority)
	HARUHI_MIKURU_COPY_ATOMIC (const_portamento_time)
	HARUHI_MIKURU_COPY_ATOMIC (amplitude_smoothing)
	HARUHI_MIKURU_COPY_ATOMIC (frequency_smoothing)
	HARUHI_MIKURU_COPY_ATOMIC (pitchbend_smoothing)
	HARUHI_MIKURU_COPY_ATOMIC (panorama_smoothing)
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


Params::Waveshaper::Waveshaper()
{
}


void
Params::Waveshaper::set_controller_params (Waveshaper& other)
{
}


void
Params::Waveshaper::set_non_controller_params (Waveshaper& other)
{
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
	enabled (1),
	direct_adsr (1),
	forced_release (0),
	sustain_enabled (1),
	function (Linear),
	mode (Polyphonic)
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
	HARUHI_MIKURU_COPY_ATOMIC (enabled)
	HARUHI_MIKURU_COPY_ATOMIC (direct_adsr)
	HARUHI_MIKURU_COPY_ATOMIC (forced_release)
	HARUHI_MIKURU_COPY_ATOMIC (sustain_enabled)
	HARUHI_MIKURU_COPY_ATOMIC (function)
	HARUHI_MIKURU_COPY_ATOMIC (mode)
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
	enabled (1),
	wave_type (Sine),
	wave_invert (0),
	function (Linear),
	mode (Polyphonic),
	tempo_sync (0),
	tempo_numerator (1),
	tempo_denominator (1),
	random_start_phase (0),
	fade_out_enabled (0)
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
	HARUHI_MIKURU_COPY_ATOMIC (enabled)
	HARUHI_MIKURU_COPY_ATOMIC (wave_type)
	HARUHI_MIKURU_COPY_ATOMIC (wave_invert)
	HARUHI_MIKURU_COPY_ATOMIC (function)
	HARUHI_MIKURU_COPY_ATOMIC (mode)
	HARUHI_MIKURU_COPY_ATOMIC (tempo_sync)
	HARUHI_MIKURU_COPY_ATOMIC (tempo_numerator)
	HARUHI_MIKURU_COPY_ATOMIC (tempo_denominator)
	HARUHI_MIKURU_COPY_ATOMIC (random_start_phase)
	HARUHI_MIKURU_COPY_ATOMIC (fade_out_enabled)
}


Params::EG::EG():
	// Controller:
	// Non-controller:
	enabled (1),
	segments (1),
	sustain_point (0)
{
	for (int i = 0; i < MaxPoints; ++i)
	{
		values[i] = 0;
		durations[i] = 0;
	}
}


void
Params::EG::set_controller_params (EG& other)
{
}


void
Params::EG::set_non_controller_params (EG& other)
{
	HARUHI_MIKURU_COPY_ATOMIC (enabled)
	HARUHI_MIKURU_COPY_ATOMIC (segments)
	HARUHI_MIKURU_COPY_ATOMIC (sustain_point)
	// Copy tables:
	for (int i = 0; i < MaxPoints; ++i)
	{
		values[i] = atomic (other.values[i]);
		durations[i] = atomic (other.durations[i]);
	}
}


void
Params::EG::sanitize()
{
	int p = atomic (segments);
	if (p < 2)
	{
		p = 2;
		atomic (segments) = p;
	}

	int s = atomic (sustain_point);
	if (s >= p)
	{
		s = p;
		atomic (sustain_point) = s;
	}

	// TODO sanitize values[]/durations[] values.
}

} // namespace MikuruPrivate

#undef HARUHI_MIKURU_COPY_ATOMIC
#undef HARUHI_MIKURU_COPY
#undef HARUHI_MIKURU_CONSTRUCT

