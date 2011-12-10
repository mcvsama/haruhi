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
		 name##Denominator,																		\
		 #name,																					\
		 1.0f * name##Min / name##Denominator,													\
		 1.0f * name##Max / name##Denominator,													\
		 shown_decimals,																		\
		 (name##Max - name##Min) / 200)

#define HARUHI_YUKI_CONSTRUCT_EXPLICIT(var, name, shown_min, shown_max, shown_decimals, step)	\
	var (name##Min,																				\
		 name##Max,																				\
		 name##Default,																			\
		 name##Denominator,																		\
		 #name,																					\
		 shown_min, shown_max, shown_decimals, step)

#define HARUHI_YUKI_DEFINE_PARAMS(klass)														\
	void																						\
	Params::klass::get_params (Haruhi::BaseParam const** tab, size_t max_entries) const			\
	{																							\
		size_t pos = 0;

#define HARUHI_YUKI_DEFINE_PARAM(param)															\
		tab[pos++] = &param;

#define HARUHI_YUKI_FINISH_DEFINITION()															\
		assert (pos == max_entries);															\
	}


namespace Yuki {

Params::Main::Main():
	HARUHI_YUKI_CONSTRUCT_EXPLICIT (volume, Volume, -std::numeric_limits<float>::infinity(), 0.0f, 2, (VolumeMax - VolumeMin) / 500),
	HARUHI_YUKI_CONSTRUCT (detune, Detune, 2),
	HARUHI_YUKI_CONSTRUCT (panorama, Panorama, 2),
	HARUHI_YUKI_CONSTRUCT (stereo_width, StereoWidth, 2),
	enabled (0, 1, 1, "enabled"),
	polyphony (0, 512, 32, "polyphony")
{
}


HARUHI_YUKI_DEFINE_PARAMS (Main)
	HARUHI_YUKI_DEFINE_PARAM (volume)
	HARUHI_YUKI_DEFINE_PARAM (detune)
	HARUHI_YUKI_DEFINE_PARAM (panorama)
	HARUHI_YUKI_DEFINE_PARAM (stereo_width)
	HARUHI_YUKI_DEFINE_PARAM (enabled)
	HARUHI_YUKI_DEFINE_PARAM (polyphony)
HARUHI_YUKI_FINISH_DEFINITION()


Params::Part::Part():
	HARUHI_YUKI_CONSTRUCT_EXPLICIT (volume, Volume, -std::numeric_limits<float>::infinity(), 0.0f, 2, (VolumeMax - VolumeMin) / 500),
	HARUHI_YUKI_CONSTRUCT (portamento_time, PortamentoTime, 2),
	HARUHI_YUKI_CONSTRUCT (phase, Phase, 2),
	HARUHI_YUKI_CONSTRUCT (noise_level, NoiseLevel, 2),
	HARUHI_YUKI_CONSTRUCT (wave_shape, WaveShape, 2),
	HARUHI_YUKI_CONSTRUCT (modulator_amplitude, ModulatorAmplitude, 2),
	HARUHI_YUKI_CONSTRUCT (modulator_index, ModulatorIndex, 0),
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
	modulator_wave_type (0, 3, 0, "modulator_wave_type")
{
	for (int i = 0; i < HarmonicsNumber; ++i)
		harmonics[i] = Haruhi::Param<int> (HarmonicMin, HarmonicMax, HarmonicDefault, QString ("harmonic[%1]").arg (i).utf8());
	for (int i = 0; i < HarmonicsNumber; ++i)
		harmonic_phases[i] = Haruhi::Param<int> (HarmonicPhaseMin, HarmonicPhaseMax, HarmonicPhaseDefault, QString ("harmonic-phase[%1]").arg (i).utf8());
}


HARUHI_YUKI_DEFINE_PARAMS (Part)
	HARUHI_YUKI_DEFINE_PARAM (volume)
	HARUHI_YUKI_DEFINE_PARAM (portamento_time)
	HARUHI_YUKI_DEFINE_PARAM (phase)
	HARUHI_YUKI_DEFINE_PARAM (noise_level)
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
	HARUHI_YUKI_DEFINE_PARAM (wave_shape)
	HARUHI_YUKI_DEFINE_PARAM (modulator_amplitude)
	HARUHI_YUKI_DEFINE_PARAM (modulator_index)
	HARUHI_YUKI_DEFINE_PARAM (modulator_shape)
	HARUHI_YUKI_DEFINE_PARAM (wave_type)
	HARUHI_YUKI_DEFINE_PARAM (modulator_type)
	HARUHI_YUKI_DEFINE_PARAM (modulator_wave_type)
	for (int k = 0; k < HarmonicsNumber; ++k)
		HARUHI_YUKI_DEFINE_PARAM (harmonics[k]);
	for (int k = 0; k < HarmonicsNumber; ++k)
		HARUHI_YUKI_DEFINE_PARAM (harmonic_phases[k]);
HARUHI_YUKI_FINISH_DEFINITION()


Params::Voice::Voice():
	HARUHI_YUKI_CONSTRUCT (adsr, Adsr, 2),
	HARUHI_YUKI_CONSTRUCT (amplitude, Amplitude, 2),
	HARUHI_YUKI_CONSTRUCT (frequency, Frequency, 2),
	HARUHI_YUKI_CONSTRUCT (panorama, Panorama, 2),
	HARUHI_YUKI_CONSTRUCT (detune, Detune, 2),
	HARUHI_YUKI_CONSTRUCT (pitchbend, Pitchbend, 2),
	HARUHI_YUKI_CONSTRUCT (velocity_sens, VelocitySens, 2),
	HARUHI_YUKI_CONSTRUCT (unison_index, UnisonIndex, 0),
	HARUHI_YUKI_CONSTRUCT (unison_spread, UnisonSpread, 2),
	HARUHI_YUKI_CONSTRUCT (unison_init, UnisonInit, 2),
	HARUHI_YUKI_CONSTRUCT (unison_noise, UnisonNoise, 2)
{
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
	HARUHI_YUKI_DEFINE_PARAM (adsr)
	HARUHI_YUKI_DEFINE_PARAM (amplitude)
	HARUHI_YUKI_DEFINE_PARAM (frequency)
HARUHI_YUKI_FINISH_DEFINITION()

} // namespace Yuki

#undef HARUHI_YUKI_CONSTRUCT
#undef HARUHI_YUKI_CONSTRUCT_EXPLICIT
#undef HARUHI_YUKI_DEFINE_PARAMS
#undef HARUHI_YUKI_DEFINE_PARAM
#undef HARUHI_YUKI_FINISH_DEFINITION

