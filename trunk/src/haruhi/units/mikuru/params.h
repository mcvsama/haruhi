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

#ifndef HARUHI__UNITS__MIKURU__PARAMS_H__INCLUDED
#define HARUHI__UNITS__MIKURU__PARAMS_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/dsp/harmonics_wave.h>


/*
 * Global macros.
 */

// Useful for ControllerProxy constructors:
#define HARUHI_MIKURU_MINMAX(klass_and_enum)									\
	Params::klass_and_enum##Min, Params::klass_and_enum##Max

// Useful for Knob constructors:
#define HARUHI_MIKURU_PARAMS_FOR_KNOB(klass_and_enum)							\
	1.0 * Params::klass_and_enum##Min / Params::klass_and_enum##Denominator,	\
	1.0 * Params::klass_and_enum##Max / Params::klass_and_enum##Denominator

// Useful for Knob constructors:
#define HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS(klass_and_enum, steps)			\
	1.0 * Params::klass_and_enum##Min / Params::klass_and_enum##Denominator,	\
	1.0 * Params::klass_and_enum##Max / Params::klass_and_enum##Denominator,	\
	(Params::klass_and_enum##Max - Params::klass_and_enum##Min) / steps

/*
 * Local macros, will be undefed.
 */

#define HARUHI_MIKURU_PARAMS_STANDARD_METHODS(klass)							\
	klass();																	\
	klass (klass& other) {														\
		operator= (other);														\
	}																			\
	klass& operator= (klass& other) {											\
		set_controller_params (other);											\
		set_non_controller_params (other);										\
		return *this;															\
	}																			\
	void set_controller_params (klass& other);									\
	void set_non_controller_params (klass& other);								\
	void sanitize();

#define HARUHI_MIKURU_PARAM(name, min, max, denominator, deflt)					\
	name##Min = min,															\
	name##Max = max,															\
	name##Denominator = denominator,											\
	name##Default = deflt,


namespace MikuruPrivate {

struct Params
{
	/**
	 * General Mikuru-specific params.
	 */
	struct General
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (General)

		enum {
			HARUHI_MIKURU_PARAM (Volume,				       0,	+1000000,	+1000000,	 +750000)
			HARUHI_MIKURU_PARAM (Panorama,				-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (Detune,				-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (StereoWidth,			       0,	+1000000,	+1000000,	+1000000)
			HARUHI_MIKURU_PARAM (InputVolume,			       0,	+1000000,	+1000000,	+1000000)
		};

		int volume;
		int detune;
		int panorama;
		int stereo_width;
		int input_volume;

		unsigned int polyphony;
		int enable_audio_input;
		int panorama_smoothing;
	};

	struct Filter
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Filter)

		enum {
			HARUHI_MIKURU_PARAM (Frequency,				       0,	+2400000,	 +100000,	 +100000)
			HARUHI_MIKURU_PARAM (Resonance,				       0,	+1000000,	 +100000,	 +100000)
			HARUHI_MIKURU_PARAM (Gain,					       0,	+2000000,	 +100000,	       0)
			HARUHI_MIKURU_PARAM (Attenuation,			       0,	+1000000,	+1000000,	+1000000)
		};

		int frequency;
		int resonance;
		int gain;
		int attenuation;

		int enabled;
		int type;
		int passes;
		int limiter_enabled;
		int frequency_smoothing;
		int resonance_smoothing;
		int gain_smoothing;
		int attenuation_smoothing;
	};

	/**
	 * Mikuru-specific params.
	 */
	struct CommonFilters
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (CommonFilters)

		enum {
			FilterConfigurationSerial	= 0,
			FilterConfigurationParallel	= 1,
		};

		int filter_configuration;
		int route_audio_input;
	};

	/**
	 * General Part-specific params.
	 */
	struct Part
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Part)

		int enabled;
	};

	/**
	 * Part filter params.
	 */
	struct PartFilters
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (PartFilters)

		enum {
			FilterConfigurationSerial	= 0,
			FilterConfigurationParallel	= 1,
		};

		int filter_configuration;
	};

	/**
	 * Part-specific params.
	 */
	struct Waveform
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Waveform)

		enum {
			HARUHI_MIKURU_PARAM (Harmonic,				-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (Phase,					-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (WaveShape,				       0,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (ModulatorAmplitude,	       0,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (ModulatorIndex,		      +1,	     +32,	      +1,	      +1)
			HARUHI_MIKURU_PARAM (ModulatorShape,		       0,	+1000000,	+1000000,	       0)

			HarmonicsNumber = DSP::HarmonicsWave::HarmonicsNumber,
		};

		int wave_shape;
		int modulator_amplitude;
		int modulator_index;
		int modulator_shape;

		unsigned int wave_type;
		unsigned int modulator_type;
		unsigned int modulator_wave_type;
		int harmonics[HarmonicsNumber];
		int phases[HarmonicsNumber];
	};

	/**
	 * Part-specific params.
	 */
	struct Oscillator
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Oscillator)

		enum {
			HARUHI_MIKURU_PARAM (Volume,				       0,	+1000000,	+1000000,	 +750000)
			HARUHI_MIKURU_PARAM (PortamentoTime,		       0,	+1000000,	 +100000,	       0)
			HARUHI_MIKURU_PARAM (Phase,					-1000000,	+1000000,	+1000000,	       0)
		};

		enum KeyPriority {
			LastPressed			= 0,
			FirstPressed		= 1,
			LowestPressed		= 2,
			HighestPressed		= 3,
		};

		int volume;
		int portamento_time;
		int phase;

		int frequency_mod_range;
		int pitchbend_enabled;
		int pitchbend_released;
		int pitchbend_up_semitones;
		int pitchbend_down_semitones;
		int transposition_semitones;
		int monophonic;
		int monophonic_retrigger;
		int monophonic_key_priority;
		int const_portamento_time;
		int amplitude_smoothing;
		int frequency_smoothing;
		int pitchbend_smoothing;
		int panorama_smoothing;
	};

	/**
	 * Voice-specific params.
	 */
	struct Voice
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Voice)

		enum {
			HARUHI_MIKURU_PARAM (Adsr,					       0,	+1000000,	+1000000,	+1000000)
			HARUHI_MIKURU_PARAM (Amplitude,				       0,	+1000000,	+1000000,	+1000000)
			HARUHI_MIKURU_PARAM (Frequency,				-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (Panorama,				-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (Detune,				-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (Pitchbend,				-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (VelocitySens,			-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (UnisonIndex,			      +1,	     +10,	      +1,	      +1)
			HARUHI_MIKURU_PARAM (UnisonSpread,			       0,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (UnisonInit,			-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (UnisonNoise,			       0,	+1000000,	+1000000,	       0)
		};

		int adsr;
		int amplitude;
		int frequency;
		int panorama;
		int detune;
		int pitchbend;
		int velocity_sens;
		int unison_index;
		int unison_spread;
		int unison_init;
		int unison_noise;
	};

	/**
	 * Waveshaper params.
	 */
	struct Waveshaper
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Waveshaper)
	};

	/**
	 * Envelope: ADSR
	 */
	struct ADSR
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (ADSR)

		enum {
			HARUHI_MIKURU_PARAM (Delay,					       0,	+1000000,	 +100000,	       0)
			HARUHI_MIKURU_PARAM (Attack,				       0,	+1000000,	 +100000,	       0)
			HARUHI_MIKURU_PARAM (AttackHold,			       0,	+1000000,	 +100000,	       0)
			HARUHI_MIKURU_PARAM (Decay,					       0,	+1000000,	 +100000,	       0)
			HARUHI_MIKURU_PARAM (Sustain,				       0,	+1000000,	+1000000,	+1000000)
			HARUHI_MIKURU_PARAM (SustainHold,			       0,	+1000000,	 +100000,	       0)
			HARUHI_MIKURU_PARAM (Release,				       0,	+1000000,	 +100000,	       0)
		};

		enum Function { LogarithmicE, Logarithmic2, Linear, Expotential2, ExpotentialE };
		enum Mode { Polyphonic, CommonKeySync };

		int delay;
		int attack;
		int attack_hold;
		int decay;
		int sustain;
		int sustain_hold;
		int release;

		int enabled;
		int direct_adsr;
		int forced_release;
		int sustain_enabled;
		int function;
		int mode;
	};

	/**
	 * Envelope: LFO
	 */
	struct LFO
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (LFO)

		enum {
			HARUHI_MIKURU_PARAM (Delay,					       0,	+1000000,	 +100000,	       0)
			HARUHI_MIKURU_PARAM (FadeIn,				       0,	+1000000,	 +100000,	       0)
			HARUHI_MIKURU_PARAM (Frequency,				       0,	+3000000,	 +100000,	 +200000)
			HARUHI_MIKURU_PARAM (Level,					-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (Depth,					       0,	+1000000,	+1000000,	 +500000)
			HARUHI_MIKURU_PARAM (Phase,					       0,	+1000000,	+1000000,	 +500000)
			HARUHI_MIKURU_PARAM (WaveShape,				       0,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (FadeOut,				       0,	+1000000,	 +100000,	 +100000)
		};

		enum Wave { Sine, Triangle, Square, Sawtooth, Pulse, RandomSquare, RandomTriangle };
		enum Function { LogarithmicE, Logarithmic2, Linear, Expotential2, ExpotentialE };
		enum Mode { Polyphonic, CommonKeySync, CommonContinuous };

		int delay;
		int fade_in;
		int frequency;
		int level;
		int depth;
		int phase;
		int wave_shape;
		int fade_out;

		int enabled;
		int wave_type;
		int wave_invert;
		int function;
		int mode;
		int tempo_sync;
		int tempo_numerator;
		int tempo_denominator;
		int random_start_phase;
		int fade_out_enabled;
	};

	/**
	 * Envelope: EG
	 */
	struct EG
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (EG)

		enum {
			HARUHI_MIKURU_PARAM (PointValue,			       0,	+1000000,	+1000000,	 +500000)
			HARUHI_MIKURU_PARAM (SegmentDuration,		       0,	+1000000,	 +100000,	       0)
		};

		int enabled;
		unsigned int segments;
		unsigned int sustain_point;
	};
};

} // namespace MikuruPrivate

#undef HARUHI_MIKURU_PARAMS_STANDARD_METHODS
#undef HARUHI_MIKURU_PARAM

#endif

