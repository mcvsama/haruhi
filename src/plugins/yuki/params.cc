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
#include <algorithm>
#include <limits>

// Qt:
#include <QDomElement>

// Haruhi:
#include <haruhi/utility/numeric.h>
#include <haruhi/dsp/modulated_wave.h>

// Local:
#include "params.h"


namespace Yuki {

Params::Main::Main():
	HARUHI_CONTROLLER_PARAM_CONSTRUCT_EXPLICIT (volume, Volume, Range<float> (-std::numeric_limits<float>::infinity(), 0.0f), 2, (VolumeMax - VolumeMin) / 500),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (panorama, Panorama, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (detune, Detune, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (stereo_width, StereoWidth, 2),
	enabled ({ 0, 1 }, 1, "enabled"),
	polyphony ({ 1, 512 }, 32, "polyphony"),
	oversampling ({ 1, 16 }, 1, "oversampling")
{ }


HARUHI_DEFINE_SAVEABLE_PARAMS (Main)
	HARUHI_DEFINE_SAVEABLE_PARAM (volume)
	HARUHI_DEFINE_SAVEABLE_PARAM (detune)
	HARUHI_DEFINE_SAVEABLE_PARAM (panorama)
	HARUHI_DEFINE_SAVEABLE_PARAM (stereo_width)
	HARUHI_DEFINE_SAVEABLE_PARAM (enabled)
	HARUHI_DEFINE_SAVEABLE_PARAM (polyphony)
	HARUHI_DEFINE_SAVEABLE_PARAM (oversampling)
HARUHI_FINISH_SAVEABLE_PARAMS_DEFINITION()


Params::Filter::Filter():
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (frequency, Frequency, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (resonance, Resonance, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (gain, Gain, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT_EXPLICIT (attenuation, Attenuation, Range<float> (-std::numeric_limits<float>::infinity(), 0.0f), 2, (AttenuationMax - AttenuationMin) / 500),
	enabled ({ 0, 1 }, 0, "enabled"),
	type ({ 0, 7 }, 0, "type"),
	stages ({ 1, 5 }, 1, "stages"),
	limiter_enabled ({ 0, 1 }, 1, "limiter_enabled")
{
	frequency.adapter()->curve = 1.0;
	frequency.adapter()->user_limit.set_min (0.04f * Params::Filter::FrequencyDenominator);
	frequency.adapter()->user_limit.set_max (22.0f * Params::Filter::FrequencyDenominator);
	resonance.adapter()->curve = 1.0;
	attenuation.adapter()->curve = 1.0;
}


HARUHI_DEFINE_SAVEABLE_PARAMS (Filter)
	HARUHI_DEFINE_SAVEABLE_PARAM (frequency)
	HARUHI_DEFINE_SAVEABLE_PARAM (resonance)
	HARUHI_DEFINE_SAVEABLE_PARAM (gain)
	HARUHI_DEFINE_SAVEABLE_PARAM (attenuation)
	HARUHI_DEFINE_SAVEABLE_PARAM (enabled)
	HARUHI_DEFINE_SAVEABLE_PARAM (type)
	HARUHI_DEFINE_SAVEABLE_PARAM (stages)
	HARUHI_DEFINE_SAVEABLE_PARAM (limiter_enabled)
HARUHI_FINISH_SAVEABLE_PARAMS_DEFINITION()


Params::Operator::Operator():
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (detune, Detune, 2),
	frequency_numerator ({ 1, 32 }, 1, "frequency_numerator"),
	frequency_denominator ({ 1, 32 }, 1, "frequency_denominator"),
	octave ({ -8, +8 }, 0, "octave")
{ }


HARUHI_DEFINE_SAVEABLE_PARAMS (Operator)
	HARUHI_DEFINE_SAVEABLE_PARAM (detune)
	HARUHI_DEFINE_SAVEABLE_PARAM (frequency_numerator)
	HARUHI_DEFINE_SAVEABLE_PARAM (frequency_denominator)
	HARUHI_DEFINE_SAVEABLE_PARAM (octave)
HARUHI_FINISH_SAVEABLE_PARAMS_DEFINITION()


Params::Voice::Voice():
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (amplitude, Amplitude, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (frequency, Frequency, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (panorama, Panorama, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (detune, Detune, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (pitchbend, Pitchbend, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (velocity_sens, VelocitySens, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT_EXPLICIT (unison_index, UnisonIndex, Range<int> (UnisonIndexMin, UnisonIndexMax), 0, 1),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (unison_spread, UnisonSpread, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (unison_init, UnisonInit, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (unison_noise, UnisonNoise, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (unison_vibrato_level, UnisonVibratoLevel, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (unison_vibrato_frequency, UnisonVibratoFrequency, 2)
{
	unison_spread.adapter()->curve = 1.0;
}


HARUHI_DEFINE_SAVEABLE_PARAMS (Voice)
	HARUHI_DEFINE_SAVEABLE_PARAM (amplitude)
	HARUHI_DEFINE_SAVEABLE_PARAM (frequency)
	HARUHI_DEFINE_SAVEABLE_PARAM (panorama)
	HARUHI_DEFINE_SAVEABLE_PARAM (detune)
	HARUHI_DEFINE_SAVEABLE_PARAM (pitchbend)
	HARUHI_DEFINE_SAVEABLE_PARAM (velocity_sens)
	HARUHI_DEFINE_SAVEABLE_PARAM (unison_index)
	HARUHI_DEFINE_SAVEABLE_PARAM (unison_spread)
	HARUHI_DEFINE_SAVEABLE_PARAM (unison_init)
	HARUHI_DEFINE_SAVEABLE_PARAM (unison_noise)
	HARUHI_DEFINE_SAVEABLE_PARAM (unison_vibrato_level)
	HARUHI_DEFINE_SAVEABLE_PARAM (unison_vibrato_frequency)
HARUHI_FINISH_SAVEABLE_PARAMS_DEFINITION()


Params::Part::Part():
	// TODO check with gcc-4.7 if it's possible to use { a, b } instead of Range<int> (a, b)
	HARUHI_CONTROLLER_PARAM_CONSTRUCT_EXPLICIT (volume, Volume, Range<float> (-std::numeric_limits<float>::infinity(), 0.0f), 2, (VolumeMax - VolumeMin) / 500),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (portamento_time, PortamentoTime, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (phase, Phase, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (noise_level, NoiseLevel, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (wave_shape, WaveShape, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (modulator_amplitude, ModulatorAmplitude, 2),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT_EXPLICIT (modulator_index, ModulatorIndex, Range<int> (ModulatorIndexMin, ModulatorIndexMax), 0, 1),
	HARUHI_CONTROLLER_PARAM_CONSTRUCT (modulator_shape, ModulatorShape, 2),
	part_enabled ({ 0, 1 }, 1, "part_enabled"),
	modulator_enabled ({ 0, 1 }, 0, "modulator_enabled"),
	wave_enabled ({ 0, 1 }, 1, "wave_enabled"),
	noise_enabled ({ 0, 1 }, 0, "noise_enabled"),
	frequency_mod_range ({ 0, 60 }, 12, "frequency_mod_range"),
	pitchbend_enabled ({ 0, 1 }, 1, "pitchbend_enabled"),
	pitchbend_up_semitones ({ 0, 60 }, 2, "pitchbend_up_semitones"),
	pitchbend_down_semitones ({ 0, 60 }, 2, "pitchbend_down_semitones"),
	transposition_semitones ({ -60, 60 }, 0, "transposition_semitones"),
	const_portamento_time ({ 0, 1 }, 1, "const_portamento_time"),
	unison_stereo ({ 0, 1 }, 1, "unison_stereo"),
	pseudo_stereo ({ 0, 1 }, 0, "pseudo_stereo"),
	wave_type ({ 0, 8 }, 0, "wave_type"),
	modulator_type ({ 0, 1 }, Haruhi::DSP::ModulatedWave::Ring, "modulator_type"),
	modulator_wave_type ({ 0, 3 }, 0, "modulator_wave_type"),
	auto_center ({ 0, 1 }, 0, "auto_center"),
	filter_configuration ({ 0, 1 }, 0, "filter_configuration")
{
	for (unsigned int i = 0; i < HarmonicsNumber; ++i)
		harmonics[i] = Haruhi::v06::ControllerParam ({ HarmonicMin, HarmonicMax }, HarmonicCenterValue, HarmonicDefault, HarmonicDenominator, QString ("harmonic[%1]").arg (i).toUtf8().data());
	for (unsigned int i = 0; i < HarmonicsNumber; ++i)
		harmonic_phases[i] = Haruhi::v06::ControllerParam ({ HarmonicPhaseMin, HarmonicPhaseMax }, HarmonicPhaseCenterValue, HarmonicPhaseDefault, HarmonicPhaseDenominator, QString ("harmonic-phase[%1]").arg (i).toUtf8().data());
	// First/base harmonic should be fully max:
	harmonics[0].set (HarmonicMax);

	// Modulator matrix:
	for (unsigned int o = 0; o < 4; ++o)
	{
		for (unsigned int i = 0; i < 3; ++i)
		{
			fm_matrix[o][i] = Haruhi::v06::ControllerParam ({ FrequencyModMin, FrequencyModMax }, FrequencyModCenterValue, FrequencyModDefault, FrequencyModDenominator,
															  QString ("fm-matrix[%1][%2]").arg (o).arg (i).toUtf8().data(), HARUHI_CONTROLLER_PARAM_ADDITIONAL_ARGS (FrequencyMod, 2));
			am_matrix[o][i] = Haruhi::v06::ControllerParam ({ AmplitudeModMin, AmplitudeModMax }, AmplitudeModCenterValue, AmplitudeModDefault, AmplitudeModDenominator,
															  QString ("am-matrix[%1][%2]").arg (o).arg (i).toUtf8().data(), HARUHI_CONTROLLER_PARAM_ADDITIONAL_ARGS (AmplitudeMod, 2));
		}
	}

	portamento_time.adapter()->curve = 1.0;
	portamento_time.adapter()->user_limit.set_max (0.5f * Params::Part::PortamentoTimeDenominator);
}


HARUHI_DEFINE_SAVEABLE_PARAMS (Part)
	HARUHI_DEFINE_SAVEABLE_PARAM (volume)
	HARUHI_DEFINE_SAVEABLE_PARAM (portamento_time)
	HARUHI_DEFINE_SAVEABLE_PARAM (phase)
	HARUHI_DEFINE_SAVEABLE_PARAM (noise_level)
	HARUHI_DEFINE_SAVEABLE_PARAM (wave_shape)
	HARUHI_DEFINE_SAVEABLE_PARAM (modulator_amplitude)
	HARUHI_DEFINE_SAVEABLE_PARAM (modulator_index)
	HARUHI_DEFINE_SAVEABLE_PARAM (modulator_shape)
	for (unsigned int k = 0; k < HarmonicsNumber; ++k)
		HARUHI_DEFINE_SAVEABLE_PARAM (harmonics[k]);
	for (unsigned int k = 0; k < HarmonicsNumber; ++k)
		HARUHI_DEFINE_SAVEABLE_PARAM (harmonic_phases[k]);
	for (unsigned int o = 0; o < 4; ++o)
	{
		for (unsigned int i = 0; i < 3; ++i)
		{
			HARUHI_DEFINE_SAVEABLE_PARAM (fm_matrix[o][i]);
			HARUHI_DEFINE_SAVEABLE_PARAM (am_matrix[o][i]);
		}
	}
	HARUHI_DEFINE_SAVEABLE_PARAM (part_enabled)
	HARUHI_DEFINE_SAVEABLE_PARAM (modulator_enabled)
	HARUHI_DEFINE_SAVEABLE_PARAM (wave_enabled)
	HARUHI_DEFINE_SAVEABLE_PARAM (noise_enabled)
	HARUHI_DEFINE_SAVEABLE_PARAM (frequency_mod_range)
	HARUHI_DEFINE_SAVEABLE_PARAM (pitchbend_enabled)
	HARUHI_DEFINE_SAVEABLE_PARAM (pitchbend_up_semitones)
	HARUHI_DEFINE_SAVEABLE_PARAM (pitchbend_down_semitones)
	HARUHI_DEFINE_SAVEABLE_PARAM (transposition_semitones)
	HARUHI_DEFINE_SAVEABLE_PARAM (const_portamento_time)
	HARUHI_DEFINE_SAVEABLE_PARAM (unison_stereo)
	HARUHI_DEFINE_SAVEABLE_PARAM (pseudo_stereo)
	HARUHI_DEFINE_SAVEABLE_PARAM (wave_type)
	HARUHI_DEFINE_SAVEABLE_PARAM (modulator_type)
	HARUHI_DEFINE_SAVEABLE_PARAM (modulator_wave_type)
	HARUHI_DEFINE_SAVEABLE_PARAM (auto_center)
	HARUHI_DEFINE_SAVEABLE_PARAM (filter_configuration)
HARUHI_FINISH_SAVEABLE_PARAMS_DEFINITION()

} // namespace Yuki

