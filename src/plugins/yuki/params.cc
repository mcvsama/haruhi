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

// Haruhi:
#include <haruhi/utility/numeric.h>
#include <haruhi/dsp/modulated_wave.h>

// Local:
#include "params.h"

// Qt:
#include <QtXml/QDomElement>


#define HARUHI_YUKI_CONSTRUCT(var, name)												\
	var (name##Min, name##Max, name##Default, name##Denominator, #name)

#define HARUHI_YUKI_DEFINE_PARAMS(klass)												\
	void																				\
	Params::klass::get_params (Haruhi::BaseParam const** tab, size_t max_entries) const	\
	{																					\
		size_t pos = 0;

#define HARUHI_YUKI_DEFINE_PARAM(param)													\
		tab[pos++] = &param;

#define HARUHI_YUKI_FINISH_DEFINITION()													\
		assert (pos == max_entries);													\
	}


namespace Yuki {

Params::MainControls::MainControls():
	HARUHI_YUKI_CONSTRUCT (volume, Volume),
	HARUHI_YUKI_CONSTRUCT (detune, Detune),
	HARUHI_YUKI_CONSTRUCT (panorama, Panorama),
	HARUHI_YUKI_CONSTRUCT (stereo_width, StereoWidth),
	enabled (0, 1, 1, "enabled"),
	polyphony (0, 512, 32, "polyphony")
{
}


HARUHI_YUKI_DEFINE_PARAMS (MainControls)
	HARUHI_YUKI_DEFINE_PARAM (volume)
	HARUHI_YUKI_DEFINE_PARAM (detune)
	HARUHI_YUKI_DEFINE_PARAM (panorama)
	HARUHI_YUKI_DEFINE_PARAM (stereo_width)
	HARUHI_YUKI_DEFINE_PARAM (enabled)
	HARUHI_YUKI_DEFINE_PARAM (polyphony)
HARUHI_YUKI_FINISH_DEFINITION()


Params::Part::Part():
	enabled (0, 1, 1, "enabled")
{
}


HARUHI_YUKI_DEFINE_PARAMS (Part)
	HARUHI_YUKI_DEFINE_PARAM (enabled)
HARUHI_YUKI_FINISH_DEFINITION()


Params::Waveform::Waveform():
	HARUHI_YUKI_CONSTRUCT (wave_shape, WaveShape),
	HARUHI_YUKI_CONSTRUCT (modulator_amplitude, ModulatorAmplitude),
	HARUHI_YUKI_CONSTRUCT (modulator_index, ModulatorIndex),
	HARUHI_YUKI_CONSTRUCT (modulator_shape, ModulatorShape),
	wave_type (0, 8, 0, "wave_type"),
	modulator_type (0, 1, Haruhi::DSP::ModulatedWave::Ring, "modulator_type"),
	modulator_wave_type (0, 3, 0, "modulator_wave_type")
{
	for (int i = 0; i < HarmonicsNumber; ++i)
		harmonics[i] = Haruhi::Param<int> (HarmonicMin, HarmonicMax, HarmonicDefault, QString ("harmonic[%1]").arg (i).utf8());
	for (int i = 0; i < HarmonicsNumber; ++i)
		phases[i] = Haruhi::Param<int> (PhaseMin, PhaseMax, PhaseDefault, QString ("phase[%1]").arg (i).utf8());
}


HARUHI_YUKI_DEFINE_PARAMS (Waveform)
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
		HARUHI_YUKI_DEFINE_PARAM (phases[k]);
HARUHI_YUKI_FINISH_DEFINITION()


Params::Oscillator::Oscillator():
	HARUHI_YUKI_CONSTRUCT (volume, Volume),
	HARUHI_YUKI_CONSTRUCT (portamento_time, PortamentoTime),
	HARUHI_YUKI_CONSTRUCT (phase, Phase),
	HARUHI_YUKI_CONSTRUCT (noise_level, NoiseLevel),
	wave_enabled (0, 1, 1, "wave_enabled"),
	noise_enabled (0, 1, 0, "noise_enabled"),
	frequency_mod_range (0, 60, 12, "frequency_mod_range"),
	pitchbend_enabled (0, 1, 1, "pitchbend_enabled"),
	pitchbend_released (0, 1, 0, "pitchbend_released"),
	pitchbend_up_semitones (0, 60, 2, "pitchbend_up_semitones"),
	pitchbend_down_semitones (0, 60, 2, "pitchbend_down_semitones"),
	transposition_semitones (-60, 60, 0, "transposition_semitones"),
	const_portamento_time (0, 1, 1, "const_portamento_time"),
	unison_stereo (0, 1, 1, "unison_stereo"),
	pseudo_stereo (0, 1, 0, "pseudo_stereo")
{
}


HARUHI_YUKI_DEFINE_PARAMS (Oscillator)
	HARUHI_YUKI_DEFINE_PARAM (volume)
	HARUHI_YUKI_DEFINE_PARAM (portamento_time)
	HARUHI_YUKI_DEFINE_PARAM (phase)
	HARUHI_YUKI_DEFINE_PARAM (noise_level)
	HARUHI_YUKI_DEFINE_PARAM (wave_enabled)
	HARUHI_YUKI_DEFINE_PARAM (noise_enabled)
	HARUHI_YUKI_DEFINE_PARAM (frequency_mod_range)
	HARUHI_YUKI_DEFINE_PARAM (pitchbend_enabled)
	HARUHI_YUKI_DEFINE_PARAM (pitchbend_released)
	HARUHI_YUKI_DEFINE_PARAM (pitchbend_up_semitones)
	HARUHI_YUKI_DEFINE_PARAM (pitchbend_down_semitones)
	HARUHI_YUKI_DEFINE_PARAM (transposition_semitones)
	HARUHI_YUKI_DEFINE_PARAM (const_portamento_time)
	HARUHI_YUKI_DEFINE_PARAM (unison_stereo)
	HARUHI_YUKI_DEFINE_PARAM (pseudo_stereo)
HARUHI_YUKI_FINISH_DEFINITION()


Params::Voice::Voice():
	HARUHI_YUKI_CONSTRUCT (adsr, Adsr),
	HARUHI_YUKI_CONSTRUCT (amplitude, Amplitude),
	HARUHI_YUKI_CONSTRUCT (frequency, Frequency),
	HARUHI_YUKI_CONSTRUCT (panorama, Panorama),
	HARUHI_YUKI_CONSTRUCT (detune, Detune),
	HARUHI_YUKI_CONSTRUCT (pitchbend, Pitchbend),
	HARUHI_YUKI_CONSTRUCT (velocity_sens, VelocitySens),
	HARUHI_YUKI_CONSTRUCT (unison_index, UnisonIndex),
	HARUHI_YUKI_CONSTRUCT (unison_spread, UnisonSpread),
	HARUHI_YUKI_CONSTRUCT (unison_init, UnisonInit),
	HARUHI_YUKI_CONSTRUCT (unison_noise, UnisonNoise)
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

