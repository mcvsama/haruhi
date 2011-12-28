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
#include <algorithm>
#include <limits>

// Haruhi:
#include <haruhi/utility/numeric.h>
#include <haruhi/dsp/modulated_wave.h>

// Local:
#include "params.h"

// Qt:
#include <QtXml/QDomElement>


#define HARUHI_YUKI_CONSTRUCT(var, name, shown_decimals)										\
	var (name##Min,																				\
		 name##Max,																				\
		 name##Default,																			\
		 name##ZeroValue,																		\
		 name##Denominator,																		\
		 #name,																					\
		 1.0f * name##Min / name##Denominator,													\
		 1.0f * name##Max / name##Denominator,													\
		 shown_decimals,																		\
		 (name##Max - name##Min) / 400)

#define HARUHI_YUKI_CONSTRUCT_EXPLICIT(var, name, shown_min, shown_max, shown_decimals, step)	\
	var (name##Min,																				\
		 name##Max,																				\
		 name##Default,																			\
		 name##ZeroValue,																		\
		 name##Denominator,																		\
		 #name,																					\
		 shown_min, shown_max, shown_decimals, step)

#define HARUHI_YUKI_ADDITIONAL_ARGS(name, shown_decimals)										\
		 1.0f * name##Min / name##Denominator,													\
		 1.0f * name##Max / name##Denominator,													\
		 shown_decimals,																		\
		 (name##Max - name##Min) / 400

#define HARUHI_YUKI_DEFINE_PARAMS(klass)														\
	void																						\
	Params::klass::get_params (Haruhi::BaseParam const** tab, std::size_t max_entries) const	\
	{																							\
		std::size_t pos = 0;

#define HARUHI_YUKI_DEFINE_PARAM(param)															\
		tab[pos++] = &param;

#define HARUHI_YUKI_FINISH_DEFINITION()															\
		assert (pos == max_entries);															\
	}


namespace Yuki {

Params::Main::Main():
	HARUHI_YUKI_CONSTRUCT_EXPLICIT (volume, Volume, -std::numeric_limits<float>::infinity(), 0.0f, 2, (VolumeMax - VolumeMin) / 500),
	HARUHI_YUKI_CONSTRUCT (panorama, Panorama, 2),
	HARUHI_YUKI_CONSTRUCT (detune, Detune, 2),
	HARUHI_YUKI_CONSTRUCT (stereo_width, StereoWidth, 2),
	enabled (0, 1, 1, "enabled"),
	polyphony (0, 512, 32, "polyphony")
{ }


HARUHI_YUKI_DEFINE_PARAMS (Main)
	HARUHI_YUKI_DEFINE_PARAM (volume)
	HARUHI_YUKI_DEFINE_PARAM (detune)
	HARUHI_YUKI_DEFINE_PARAM (panorama)
	HARUHI_YUKI_DEFINE_PARAM (stereo_width)
	HARUHI_YUKI_DEFINE_PARAM (enabled)
	HARUHI_YUKI_DEFINE_PARAM (polyphony)
HARUHI_YUKI_FINISH_DEFINITION()


Params::Filter::Filter():
	HARUHI_YUKI_CONSTRUCT (frequency, Frequency, 2),
	HARUHI_YUKI_CONSTRUCT (resonance, Resonance, 2),
	HARUHI_YUKI_CONSTRUCT (gain, Gain, 2),
	HARUHI_YUKI_CONSTRUCT_EXPLICIT (attenuation, Attenuation, -std::numeric_limits<float>::infinity(), 0.0f, 2, (AttenuationMax - AttenuationMin) / 500),
	enabled (0, 1, 0, "enabled"),
	type (0, 7, 0, "type"),
	stages (1, 5, 1, "stages"),
	limiter_enabled (0, 1, 1, "limiter_enabled")
{
	frequency.adapter()->curve = 1.0;
	frequency.adapter()->user_limit_min = 0.04 * Params::Filter::FrequencyDenominator;
	frequency.adapter()->user_limit_max = 22.0 * Params::Filter::FrequencyDenominator;
	resonance.adapter()->curve = 1.0;
	attenuation.adapter()->curve = 1.0;
}


HARUHI_YUKI_DEFINE_PARAMS (Filter)
	HARUHI_YUKI_DEFINE_PARAM (frequency)
	HARUHI_YUKI_DEFINE_PARAM (resonance)
	HARUHI_YUKI_DEFINE_PARAM (gain)
	HARUHI_YUKI_DEFINE_PARAM (attenuation)
	HARUHI_YUKI_DEFINE_PARAM (enabled)
	HARUHI_YUKI_DEFINE_PARAM (type)
	HARUHI_YUKI_DEFINE_PARAM (stages)
	HARUHI_YUKI_DEFINE_PARAM (limiter_enabled)
HARUHI_YUKI_FINISH_DEFINITION()


Params::Voice::Voice():
	HARUHI_YUKI_CONSTRUCT (amplitude, Amplitude, 2),
	HARUHI_YUKI_CONSTRUCT (frequency, Frequency, 2),
	HARUHI_YUKI_CONSTRUCT (panorama, Panorama, 2),
	HARUHI_YUKI_CONSTRUCT (detune, Detune, 2),
	HARUHI_YUKI_CONSTRUCT (pitchbend, Pitchbend, 2),
	HARUHI_YUKI_CONSTRUCT (velocity_sens, VelocitySens, 2),
	HARUHI_YUKI_CONSTRUCT_EXPLICIT (unison_index, UnisonIndex, UnisonIndexMin, UnisonIndexMax, 0, 1),
	HARUHI_YUKI_CONSTRUCT (unison_spread, UnisonSpread, 2),
	HARUHI_YUKI_CONSTRUCT (unison_init, UnisonInit, 2),
	HARUHI_YUKI_CONSTRUCT (unison_noise, UnisonNoise, 2),
	HARUHI_YUKI_CONSTRUCT (unison_vibrato_level, UnisonVibratoLevel, 2),
	HARUHI_YUKI_CONSTRUCT (unison_vibrato_frequency, UnisonVibratoFrequency, 2)
{
	unison_spread.adapter()->curve = 1.0;
}


HARUHI_YUKI_DEFINE_PARAMS (Voice)
	HARUHI_YUKI_DEFINE_PARAM (panorama)
	HARUHI_YUKI_DEFINE_PARAM (detune)
	HARUHI_YUKI_DEFINE_PARAM (pitchbend)
	HARUHI_YUKI_DEFINE_PARAM (velocity_sens)
	HARUHI_YUKI_DEFINE_PARAM (unison_index)
	HARUHI_YUKI_DEFINE_PARAM (unison_spread)
	HARUHI_YUKI_DEFINE_PARAM (unison_init)
	HARUHI_YUKI_DEFINE_PARAM (unison_noise)
	HARUHI_YUKI_DEFINE_PARAM (unison_vibrato_level)
	HARUHI_YUKI_DEFINE_PARAM (unison_vibrato_frequency)
	HARUHI_YUKI_DEFINE_PARAM (amplitude)
	HARUHI_YUKI_DEFINE_PARAM (frequency)
HARUHI_YUKI_FINISH_DEFINITION()


Params::Part::Part():
	HARUHI_YUKI_CONSTRUCT_EXPLICIT (volume, Volume, -std::numeric_limits<float>::infinity(), 0.0f, 2, (VolumeMax - VolumeMin) / 500),
	HARUHI_YUKI_CONSTRUCT (portamento_time, PortamentoTime, 2),
	HARUHI_YUKI_CONSTRUCT (phase, Phase, 2),
	HARUHI_YUKI_CONSTRUCT (noise_level, NoiseLevel, 2),
	HARUHI_YUKI_CONSTRUCT (wave_shape, WaveShape, 2),
	HARUHI_YUKI_CONSTRUCT (modulator_amplitude, ModulatorAmplitude, 2),
	HARUHI_YUKI_CONSTRUCT_EXPLICIT (modulator_index, ModulatorIndex, ModulatorIndexMin, ModulatorIndexMax, 0, 1),
	HARUHI_YUKI_CONSTRUCT (modulator_shape, ModulatorShape, 2),
	part_enabled (0, 1, 1, "part_enabled"),
	wave_enabled (0, 1, 1, "wave_enabled"),
	noise_enabled (0, 1, 0, "noise_enabled"),
	frequency_mod_range (0, 60, 12, "frequency_mod_range"),
	pitchbend_enabled (0, 1, 1, "pitchbend_enabled"),
	pitchbend_up_semitones (0, 60, 2, "pitchbend_up_semitones"),
	pitchbend_down_semitones (0, 60, 2, "pitchbend_down_semitones"),
	transposition_semitones (-60, 60, 0, "transposition_semitones"),
	const_portamento_time (0, 1, 1, "const_portamento_time"),
	unison_stereo (0, 1, 1, "unison_stereo"),
	pseudo_stereo (0, 1, 0, "pseudo_stereo"),
	wave_type (0, 8, 0, "wave_type"),
	modulator_type (0, 1, Haruhi::DSP::ModulatedWave::Ring, "modulator_type"),
	modulator_wave_type (0, 3, 0, "modulator_wave_type"),
	auto_center (0, 1, 0, "auto_center"),
	filter_configuration (0, 1, 0, "filter_configuration")
{
	for (unsigned int i = 0; i < HarmonicsNumber; ++i)
		harmonics[i] = Haruhi::ControllerParam (HarmonicMin, HarmonicMax, HarmonicZeroValue, HarmonicDefault, HarmonicDenominator, QString ("harmonic[%1]").arg (i).utf8());
	for (unsigned int i = 0; i < HarmonicsNumber; ++i)
		harmonic_phases[i] = Haruhi::ControllerParam (HarmonicPhaseMin, HarmonicPhaseMax, HarmonicPhaseZeroValue, HarmonicPhaseDefault, HarmonicPhaseDenominator, QString ("harmonic-phase[%1]").arg (i).utf8());
	// First/base harmonic should be fully max:
	harmonics[0].set (HarmonicMax);

	// Modulator matrix:
	for (unsigned int o = 0; o < 4; ++o)
	{
		for (unsigned int i = 0; i < 3; ++i)
		{
			fm_matrix[o][i] = Haruhi::ControllerParam (FrequencyModMin, FrequencyModMax, FrequencyModZeroValue, FrequencyModDefault, FrequencyModDenominator,
													   QString ("fm-matrix[%1][%2]").arg (o).arg (i).utf8(), HARUHI_YUKI_ADDITIONAL_ARGS (FrequencyMod, 2));
			am_matrix[o][i] = Haruhi::ControllerParam (AmplitudeModMin, AmplitudeModMax, AmplitudeModZeroValue, AmplitudeModDefault, AmplitudeModDenominator,
													   QString ("am-matrix[%1][%2]").arg (o).arg (i).utf8(), HARUHI_YUKI_ADDITIONAL_ARGS (AmplitudeMod, 2));
		}
	}

	portamento_time.adapter()->curve = 1.0;
	portamento_time.adapter()->user_limit_max = 0.5f * Params::Part::PortamentoTimeDenominator;
}


HARUHI_YUKI_DEFINE_PARAMS (Part)
	HARUHI_YUKI_DEFINE_PARAM (volume)
	HARUHI_YUKI_DEFINE_PARAM (portamento_time)
	HARUHI_YUKI_DEFINE_PARAM (phase)
	HARUHI_YUKI_DEFINE_PARAM (noise_level)
	HARUHI_YUKI_DEFINE_PARAM (wave_shape)
	HARUHI_YUKI_DEFINE_PARAM (modulator_amplitude)
	HARUHI_YUKI_DEFINE_PARAM (modulator_index)
	HARUHI_YUKI_DEFINE_PARAM (modulator_shape)
	for (unsigned int k = 0; k < HarmonicsNumber; ++k)
		HARUHI_YUKI_DEFINE_PARAM (harmonics[k]);
	for (unsigned int k = 0; k < HarmonicsNumber; ++k)
		HARUHI_YUKI_DEFINE_PARAM (harmonic_phases[k]);
	HARUHI_YUKI_DEFINE_PARAM (part_enabled)
	HARUHI_YUKI_DEFINE_PARAM (wave_enabled)
	HARUHI_YUKI_DEFINE_PARAM (noise_enabled)
	HARUHI_YUKI_DEFINE_PARAM (frequency_mod_range)
	HARUHI_YUKI_DEFINE_PARAM (pitchbend_enabled)
	HARUHI_YUKI_DEFINE_PARAM (pitchbend_up_semitones)
	HARUHI_YUKI_DEFINE_PARAM (pitchbend_down_semitones)
	HARUHI_YUKI_DEFINE_PARAM (transposition_semitones)
	HARUHI_YUKI_DEFINE_PARAM (const_portamento_time)
	HARUHI_YUKI_DEFINE_PARAM (unison_stereo)
	HARUHI_YUKI_DEFINE_PARAM (pseudo_stereo)
	HARUHI_YUKI_DEFINE_PARAM (wave_type)
	HARUHI_YUKI_DEFINE_PARAM (modulator_type)
	HARUHI_YUKI_DEFINE_PARAM (modulator_wave_type)
	HARUHI_YUKI_DEFINE_PARAM (auto_center)
	HARUHI_YUKI_DEFINE_PARAM (filter_configuration)
HARUHI_YUKI_FINISH_DEFINITION()

} // namespace Yuki

#undef HARUHI_YUKI_CONSTRUCT
#undef HARUHI_YUKI_CONSTRUCT_EXPLICIT
#undef HARUHI_YUKI_DEFINE_PARAMS
#undef HARUHI_YUKI_DEFINE_PARAM
#undef HARUHI_YUKI_FINISH_DEFINITION

