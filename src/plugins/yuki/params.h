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

#ifndef HARUHI__PLUGINS__YUKI__PARAMS_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PARAMS_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Haruhi:
#include <haruhi/dsp/harmonics_wave.h>
#include <haruhi/lib/controller_param.h>
#include <haruhi/lib/param.h>
#include <haruhi/plugin/saveable_params.h>


namespace Yuki {

struct Params
{
	/**
	 * Main control panel params.
	 */
	struct Main: public Haruhi::SaveableParams<Main>
	{
		HARUHI_SAVEABLE_PARAMS_STANDARD_METHODS (Main)

		HARUHI_CONTROLLER_PARAM (Volume,					       0,	+1000000,	 +938445,	       0,	+1000000) // Default: -1.5dB/20*log_10/exp
		HARUHI_CONTROLLER_PARAM (Panorama,					-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (Detune,					-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (StereoWidth,				-1000000,	+1000000,	+1000000,	       0,	+1000000)

		Haruhi::ControllerParam volume;
		Haruhi::ControllerParam panorama;
		Haruhi::ControllerParam detune;
		Haruhi::ControllerParam stereo_width;

		Haruhi::Param<int> enabled;
		Haruhi::Param<unsigned int> polyphony;

		static const std::size_t NUM_PARAMS = 6;
	};

	/**
	 * Filter parameters.
	 */
	struct Filter: public Haruhi::SaveableParams<Filter>
	{
		typedef Haruhi::ControllerParam Filter::* ControllerParamPtr;
		typedef Haruhi::Param<int> Filter::* IntParamPtr;

		HARUHI_SAVEABLE_PARAMS_STANDARD_METHODS (Filter)

		HARUHI_CONTROLLER_PARAM (Frequency,					       0,	+2400000,	 +100000,	       0,	 +100000)
		HARUHI_CONTROLLER_PARAM (Resonance,					       0,	+1000000,	 +100000,	       0,	 +100000)
		HARUHI_CONTROLLER_PARAM (Gain,						       0,	+2000000,	       0,	       0,	 +100000)
		HARUHI_CONTROLLER_PARAM (Attenuation,				       0,	+1000000,	+1000000,	       0,	+1000000)

		Haruhi::ControllerParam frequency;
		Haruhi::ControllerParam resonance;
		Haruhi::ControllerParam gain;
		Haruhi::ControllerParam attenuation;

		Haruhi::Param<int> enabled;
		Haruhi::Param<int> type;
		Haruhi::Param<int> stages;
		Haruhi::Param<int> limiter_enabled;

		static const std::size_t NUM_PARAMS = 8;
	};

	/**
	 * Operator parameters.
	 */
	struct Operator: public Haruhi::SaveableParams<Operator>
	{
		HARUHI_SAVEABLE_PARAMS_STANDARD_METHODS (Operator)

		HARUHI_CONTROLLER_PARAM (Detune,					-1200000,	+1200000,	       0,	       0,	 +100000)
		HARUHI_CONTROLLER_PARAM (Phase,						-1000000,	+1000000,	       0,	       0,	+1000000)

		Haruhi::ControllerParam detune;

		Haruhi::Param<unsigned int> frequency_numerator;
		Haruhi::Param<unsigned int> frequency_denominator;
		Haruhi::Param<int> octave;

		static const std::size_t NUM_PARAMS = 4;
	};

	/**
	 * Voice-specific params.
	 */
	struct Voice: public Haruhi::SaveableParams<Voice>
	{
		typedef Haruhi::ControllerParam Voice::* ControllerParamPtr;

		HARUHI_SAVEABLE_PARAMS_STANDARD_METHODS (Voice)

		HARUHI_CONTROLLER_PARAM (Amplitude,					       0,	+1000000,	+1000000,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (Frequency,					-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (Panorama,					-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (Detune,					-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (Pitchbend,					-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (VelocitySens,				-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (UnisonIndex,				      +1,	     +10,	      +1,	      +1,	      +1)
		HARUHI_CONTROLLER_PARAM (UnisonSpread,				       0,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (UnisonInit,				-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (UnisonNoise,				       0,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (UnisonVibratoLevel,		       0,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (UnisonVibratoFrequency,	       0,	+1000000,	 +200000,	       0,	 +100000)

		static const std::size_t FiltersNumber = 2;

		Haruhi::ControllerParam amplitude;
		Haruhi::ControllerParam frequency;
		Haruhi::ControllerParam panorama;
		Haruhi::ControllerParam detune;
		Haruhi::ControllerParam pitchbend;
		Haruhi::ControllerParam velocity_sens;
		Haruhi::ControllerParam unison_index;
		Haruhi::ControllerParam unison_spread;
		Haruhi::ControllerParam unison_init;
		Haruhi::ControllerParam unison_noise;
		Haruhi::ControllerParam unison_vibrato_level;
		Haruhi::ControllerParam unison_vibrato_frequency;

		static const std::size_t NUM_PARAMS = 12;

		// Embedded dual Filter params:
		Filter filters[FiltersNumber];
	};

	/**
	 * General Part-specific params.
	 */
	struct Part: public Haruhi::SaveableParams<Part>
	{
		HARUHI_SAVEABLE_PARAMS_STANDARD_METHODS (Part)

		HARUHI_CONTROLLER_PARAM (Volume,					       0,	+1000000,	 +938445,	       0,	+1000000) // Default: -1.5dB/20*log_10/exp
		HARUHI_CONTROLLER_PARAM (PortamentoTime,			       0,	+1000000,	       0,	       0,	 +100000)
		HARUHI_CONTROLLER_PARAM (Phase,						-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (NoiseLevel,				       0,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (WaveShape,					       0,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (ModulatorAmplitude,		       0,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (ModulatorIndex,			      +1,	     +32,	      +1,	      +1,	      +1)
		HARUHI_CONTROLLER_PARAM (ModulatorShape,			       0,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (Harmonic,					-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (HarmonicPhase,				-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (AmplitudeMod,				-1000000,	+1000000,	       0,	       0,	+1000000)
		HARUHI_CONTROLLER_PARAM (FrequencyMod,				-1000000,	+1000000,	       0,	       0,	 +100000)

		static const std::size_t HarmonicsNumber = Haruhi::DSP::HarmonicsWave::HarmonicsNumber;
		static const std::size_t OperatorsNumber = 3;

		Haruhi::ControllerParam volume;
		Haruhi::ControllerParam portamento_time;
		Haruhi::ControllerParam phase;
		Haruhi::ControllerParam noise_level;
		Haruhi::ControllerParam wave_shape;
		Haruhi::ControllerParam modulator_amplitude;
		Haruhi::ControllerParam modulator_index;
		Haruhi::ControllerParam modulator_shape;
		Haruhi::ControllerParam harmonics[HarmonicsNumber];
		Haruhi::ControllerParam harmonic_phases[HarmonicsNumber];
		Haruhi::ControllerParam fm_matrix[4][3];
		Haruhi::ControllerParam am_matrix[4][3];

		Haruhi::Param<int> part_enabled;
		Haruhi::Param<int> modulator_enabled;
		Haruhi::Param<int> wave_enabled;
		Haruhi::Param<int> noise_enabled;
		Haruhi::Param<int> frequency_mod_range;
		Haruhi::Param<int> pitchbend_enabled;
		Haruhi::Param<int> pitchbend_up_semitones;
		Haruhi::Param<int> pitchbend_down_semitones;
		Haruhi::Param<int> transposition_semitones;
		Haruhi::Param<int> const_portamento_time;
		Haruhi::Param<int> unison_stereo;
		Haruhi::Param<int> pseudo_stereo;
		Haruhi::Param<unsigned int> wave_type;
		Haruhi::Param<unsigned int> modulator_type;
		Haruhi::Param<unsigned int> modulator_wave_type;
		Haruhi::Param<unsigned int> auto_center;
		Haruhi::Param<unsigned int> filter_configuration;

		static const std::size_t NUM_PARAMS = 25 + HarmonicsNumber + HarmonicsNumber + 24;

		// Embedded Voice params template (also includes Filter params):
		Voice voice;
		Operator operators[OperatorsNumber];
	};
};

} // namespace Yuki

#endif

