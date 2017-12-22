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

#ifndef HARUHI__PLUGINS__YUKI__PARAMS_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PARAMS_H__INCLUDED

// Standard:
#include <cstddef>

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

		HARUHI_CONTROLLER_PARAM (Volume,					         0,	+1'000'000,	+1'000'000 - 1.5_dB,	+1'000'000,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (Panorama,					-1'000'000,	+1'000'000,	                  0,	         0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (Detune,					-1'000'000,	+1'000'000,	                  0,	         0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (StereoWidth,				-1'000'000,	+1'000'000,	         +1'000'000,	         0,	+1'000'000)

		Haruhi::v06::ControllerParam volume;
		Haruhi::v06::ControllerParam panorama;
		Haruhi::v06::ControllerParam detune;
		Haruhi::v06::ControllerParam stereo_width;

		Haruhi::v06::Param<int> enabled;
		Haruhi::v06::Param<unsigned int> polyphony;
		Haruhi::v06::Param<unsigned int> oversampling;

		static const std::size_t NUM_PARAMS = 7;
	};

	/**
	 * Filter parameters.
	 */
	struct Filter: public Haruhi::SaveableParams<Filter>
	{
		typedef Haruhi::v06::ControllerParam Filter::* ControllerParamPtr;
		typedef Haruhi::v06::Param<int> Filter::* IntParamPtr;

		HARUHI_SAVEABLE_PARAMS_STANDARD_METHODS (Filter)

		HARUHI_CONTROLLER_PARAM (Frequency,					       0,	+2'400'000,	  +100'000,	       0,	  +100'000)
		HARUHI_CONTROLLER_PARAM (Resonance,					       0,	+1'000'000,	  +100'000,	       0,	  +100'000)
		HARUHI_CONTROLLER_PARAM (Gain,						       0,	+2'000'000,	         0,	       0,	  +100'000)
		HARUHI_CONTROLLER_PARAM (Attenuation,				       0,	+1'000'000,	+1'000'000,	       0,	+1'000'000)

		Haruhi::v06::ControllerParam frequency;
		Haruhi::v06::ControllerParam resonance;
		Haruhi::v06::ControllerParam gain;
		Haruhi::v06::ControllerParam attenuation;

		Haruhi::v06::Param<int> enabled;
		Haruhi::v06::Param<int> type;
		Haruhi::v06::Param<int> stages;
		Haruhi::v06::Param<int> limiter_enabled;

		static const std::size_t NUM_PARAMS = 8;
	};

	/**
	 * Operator parameters.
	 */
	struct Operator: public Haruhi::SaveableParams<Operator>
	{
		HARUHI_SAVEABLE_PARAMS_STANDARD_METHODS (Operator)

		HARUHI_CONTROLLER_PARAM (Detune,					-1'200'000,	+1'200'000,	       0,	       0,	  +100'000)
		HARUHI_CONTROLLER_PARAM (Phase,						-1'000'000,	+1'000'000,	       0,	       0,	+1'000'000)

		Haruhi::v06::ControllerParam detune;

		Haruhi::v06::Param<unsigned int> frequency_numerator;
		Haruhi::v06::Param<unsigned int> frequency_denominator;
		Haruhi::v06::Param<int> octave;

		static const std::size_t NUM_PARAMS = 4;
	};

	/**
	 * Voice-specific params.
	 */
	struct Voice: public Haruhi::SaveableParams<Voice>
	{
		typedef Haruhi::v06::ControllerParam Voice::* ControllerParamPtr;

		HARUHI_SAVEABLE_PARAMS_STANDARD_METHODS (Voice)

		HARUHI_CONTROLLER_PARAM (Amplitude,					         0,	+1'000'000,	+1'000'000,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (Frequency,					-1'000'000,	+1'000'000,	         0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (Panorama,					-1'000'000,	+1'000'000,	         0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (Detune,					-1'000'000,	+1'000'000,	         0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (Pitchbend,					-1'000'000,	+1'000'000,	         0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (VelocitySens,				-1'000'000,	+1'000'000,	         0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (UnisonIndex,				        +1,	       +10,	        +1,	      +1,	        +1)
		HARUHI_CONTROLLER_PARAM (UnisonSpread,				         0,	+1'000'000,	         0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (UnisonInit,				-1'000'000,	+1'000'000,	         0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (UnisonNoise,				         0,	+1'000'000,	         0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (UnisonVibratoLevel,		         0,	+1'000'000,	         0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (UnisonVibratoFrequency,	         0,	+1'000'000,	  +200'000,	       0,	  +100'000)

		static const std::size_t FiltersNumber = 2;

		Haruhi::v06::ControllerParam amplitude;
		Haruhi::v06::ControllerParam frequency;
		Haruhi::v06::ControllerParam panorama;
		Haruhi::v06::ControllerParam detune;
		Haruhi::v06::ControllerParam pitchbend;
		Haruhi::v06::ControllerParam velocity_sens;
		Haruhi::v06::ControllerParam unison_index;
		Haruhi::v06::ControllerParam unison_spread;
		Haruhi::v06::ControllerParam unison_init;
		Haruhi::v06::ControllerParam unison_noise;
		Haruhi::v06::ControllerParam unison_vibrato_level;
		Haruhi::v06::ControllerParam unison_vibrato_frequency;

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

		HARUHI_CONTROLLER_PARAM (Volume,					         0,	+1'000'000,	+1'000'000 - 1.5_dB,	       0,	+1'000'000) // Default: -1.5dB/20*log_10/exp
		HARUHI_CONTROLLER_PARAM (PortamentoTime,			         0,	+1'000'000,	                  0,	       0,	  +100'000)
		HARUHI_CONTROLLER_PARAM (Phase,						-1'000'000,	+1'000'000,	                  0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (NoiseLevel,				         0,	+1'000'000,	                  0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (WaveShape,					         0,	+1'000'000,	                  0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (ModulatorAmplitude,		         0,	+1'000'000,	                  0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (ModulatorIndex,			        +1,	       +32,	                 +1,	      +1,	        +1)
		HARUHI_CONTROLLER_PARAM (ModulatorShape,			         0,	+1'000'000,	                  0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (Harmonic,					-1'000'000,	+1'000'000,	                  0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (HarmonicPhase,				-1'000'000,	+1'000'000,	                  0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (AmplitudeMod,				-1'000'000,	+1'000'000,	                  0,	       0,	+1'000'000)
		HARUHI_CONTROLLER_PARAM (FrequencyMod,				-1'000'000,	+1'000'000,	                  0,	       0,	  +100'000)

		static const std::size_t HarmonicsNumber = Haruhi::DSP::HarmonicsWave::HarmonicsNumber;
		static const std::size_t OperatorsNumber = 3;

		Haruhi::v06::ControllerParam volume;
		Haruhi::v06::ControllerParam portamento_time;
		Haruhi::v06::ControllerParam phase;
		Haruhi::v06::ControllerParam noise_level;
		Haruhi::v06::ControllerParam wave_shape;
		Haruhi::v06::ControllerParam modulator_amplitude;
		Haruhi::v06::ControllerParam modulator_index;
		Haruhi::v06::ControllerParam modulator_shape;
		Haruhi::v06::ControllerParam harmonics[HarmonicsNumber];
		Haruhi::v06::ControllerParam harmonic_phases[HarmonicsNumber];
		Haruhi::v06::ControllerParam fm_matrix[4][3];
		Haruhi::v06::ControllerParam am_matrix[4][3];

		Haruhi::v06::Param<int> part_enabled;
		Haruhi::v06::Param<int> modulator_enabled;
		Haruhi::v06::Param<int> wave_enabled;
		Haruhi::v06::Param<int> noise_enabled;
		Haruhi::v06::Param<int> frequency_mod_range;
		Haruhi::v06::Param<int> pitchbend_enabled;
		Haruhi::v06::Param<int> pitchbend_up_semitones;
		Haruhi::v06::Param<int> pitchbend_down_semitones;
		Haruhi::v06::Param<int> transposition_semitones;
		Haruhi::v06::Param<int> const_portamento_time;
		Haruhi::v06::Param<int> unison_stereo;
		Haruhi::v06::Param<int> pseudo_stereo;
		Haruhi::v06::Param<unsigned int> wave_type;
		Haruhi::v06::Param<unsigned int> modulator_type;
		Haruhi::v06::Param<unsigned int> modulator_wave_type;
		Haruhi::v06::Param<unsigned int> auto_center;
		Haruhi::v06::Param<unsigned int> filter_configuration;

		static const std::size_t NUM_PARAMS = 25 + HarmonicsNumber + HarmonicsNumber + 24;

		// Embedded Voice params template (also includes Filter params):
		Voice voice;
		Operator operators[OperatorsNumber];
	};
};

} // namespace Yuki

#endif

