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

#ifndef HARUHI__PLUGINS__MIKURU__PARAMS_H__INCLUDED
#define HARUHI__PLUGINS__MIKURU__PARAMS_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/dsp/harmonics_wave.h>
#include <haruhi/lib/controller_param.h>
#include <haruhi/lib/param.h>


/*
 * Global/external macros.
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
	virtual ~klass() { }														\
	klass (klass& other) {														\
		operator= (other);														\
	}																			\
	virtual klass& operator= (klass& other) {									\
		set_controller_params (other);											\
		set_non_controller_params (other);										\
		return *this;															\
	}																			\
	virtual void set_controller_params (klass& other);							\
	virtual void set_non_controller_params (klass& other);						\
	virtual void sanitize();

#define HARUHI_MIKURU_PARAM(name, min, max, denominator, deflt)					\
	name##Min = min,															\
	name##Max = max,															\
	name##Denominator = denominator,											\
	name##Default = deflt,


namespace DSP = Haruhi::DSP;

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
			HARUHI_MIKURU_PARAM (Volume,				       0,	+1000000,	+1000000,	 +938445) // -1.5dB/20*log_10/exp
			HARUHI_MIKURU_PARAM (Panorama,				-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (Detune,				-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (StereoWidth,			       0,	+1000000,	+1000000,	+1000000)
			HARUHI_MIKURU_PARAM (InputVolume,			       0,	+1000000,	+1000000,	+1000000)
		};

		Haruhi::ControllerParam volume;
		Haruhi::ControllerParam detune;
		Haruhi::ControllerParam panorama;
		Haruhi::ControllerParam stereo_width;
		Haruhi::ControllerParam input_volume;

		Haruhi::Param<unsigned int> polyphony;
		Haruhi::Param<int> enable_audio_input;
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

		Haruhi::ControllerParam frequency;
		Haruhi::ControllerParam resonance;
		Haruhi::ControllerParam gain;
		Haruhi::ControllerParam attenuation;

		Haruhi::Param<int> enabled;
		Haruhi::Param<int> type;
		Haruhi::Param<int> stages;
		Haruhi::Param<int> limiter_enabled;
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

		Haruhi::Param<int> filter_configuration;
		Haruhi::Param<int> route_audio_input;
	};

	/**
	 * General Part-specific params.
	 */
	struct Part
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Part)

		Haruhi::Param<int> enabled;
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

		Haruhi::Param<int> filter_configuration;
	};

	/**
	 * Part-specific params.
	 */
	struct Waveform
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Waveform)

		enum {
			HARUHI_MIKURU_PARAM (WaveShape,				       0,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (ModulatorAmplitude,	       0,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (ModulatorIndex,		      +1,	     +32,	      +1,	      +1)
			HARUHI_MIKURU_PARAM (ModulatorShape,		       0,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (Harmonic,				-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (Phase,					-1000000,	+1000000,	+1000000,	       0)

			HarmonicsNumber = DSP::HarmonicsWave::HarmonicsNumber,
		};

		Haruhi::ControllerParam wave_shape;
		Haruhi::ControllerParam modulator_amplitude;
		Haruhi::ControllerParam modulator_index;
		Haruhi::ControllerParam modulator_shape;

		Haruhi::Param<unsigned int> wave_type;
		Haruhi::Param<unsigned int> modulator_type;
		Haruhi::Param<unsigned int> modulator_wave_type;
		Haruhi::Param<int> harmonics[HarmonicsNumber];
		Haruhi::Param<int> phases[HarmonicsNumber];
	};

	/**
	 * Part-specific params.
	 */
	struct Oscillator
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Oscillator)

		enum {
			HARUHI_MIKURU_PARAM (Volume,				       0,	+1000000,	+1000000,	 +938445) // -1.5dB/20*log_10/exp
			HARUHI_MIKURU_PARAM (PortamentoTime,		       0,	+1000000,	 +100000,	       0)
			HARUHI_MIKURU_PARAM (Phase,					-1000000,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (NoiseLevel,			       0,	+1000000,	+1000000,	       0)
		};

		enum KeyPriority {
			LastPressed			= 0,
			FirstPressed		= 1,
			LowestPressed		= 2,
			HighestPressed		= 3,
		};

		Haruhi::ControllerParam volume;
		Haruhi::ControllerParam portamento_time;
		Haruhi::ControllerParam phase;
		Haruhi::ControllerParam noise_level;

		Haruhi::Param<int> wave_enabled;
		Haruhi::Param<int> noise_enabled;
		Haruhi::Param<int> frequency_mod_range;
		Haruhi::Param<int> pitchbend_enabled;
		Haruhi::Param<int> pitchbend_released;
		Haruhi::Param<int> pitchbend_up_semitones;
		Haruhi::Param<int> pitchbend_down_semitones;
		Haruhi::Param<int> transposition_semitones;
		Haruhi::Param<int> monophonic;
		Haruhi::Param<int> monophonic_retrigger;
		Haruhi::Param<int> monophonic_key_priority;
		Haruhi::Param<int> const_portamento_time;
		Haruhi::Param<int> unison_stereo;
		Haruhi::Param<int> pseudo_stereo;
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

		Haruhi::ControllerParam adsr;
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

		Haruhi::ControllerParam delay;
		Haruhi::ControllerParam attack;
		Haruhi::ControllerParam attack_hold;
		Haruhi::ControllerParam decay;
		Haruhi::ControllerParam sustain;
		Haruhi::ControllerParam sustain_hold;
		Haruhi::ControllerParam release;

		Haruhi::Param<int> enabled;
		Haruhi::Param<int> direct_adsr;
		Haruhi::Param<int> forced_release;
		Haruhi::Param<int> sustain_enabled;
		Haruhi::Param<int> function;
		Haruhi::Param<int> mode;
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

		Haruhi::ControllerParam delay;
		Haruhi::ControllerParam fade_in;
		Haruhi::ControllerParam frequency;
		Haruhi::ControllerParam level;
		Haruhi::ControllerParam depth;
		Haruhi::ControllerParam phase;
		Haruhi::ControllerParam wave_shape;
		Haruhi::ControllerParam fade_out;

		Haruhi::Param<int> enabled;
		Haruhi::Param<int> wave_type;
		Haruhi::Param<int> wave_invert;
		Haruhi::Param<int> function;
		Haruhi::Param<int> mode;
		Haruhi::Param<int> tempo_sync;
		Haruhi::Param<int> tempo_numerator;
		Haruhi::Param<int> tempo_denominator;
		Haruhi::Param<int> random_start_phase;
		Haruhi::Param<int> fade_out_enabled;
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

		enum { MaxPoints = 64 };

		Haruhi::Param<int> enabled;
		Haruhi::Param<unsigned int> segments;
		Haruhi::Param<unsigned int> sustain_point;
		Haruhi::Param<unsigned int> values[MaxPoints];
		Haruhi::Param<unsigned int> durations[MaxPoints];
	};

	/**
	 * Base class for effects params
	 */
	struct Effect
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Effect)

		enum {
			HARUHI_MIKURU_PARAM (Wet,					       0,	+1000000,	+1000000,	 +500000)
			HARUHI_MIKURU_PARAM (Panorama,				-1000000,	+1000000,	+1000000,	       0)
		};

		Haruhi::ControllerParam wet;
		Haruhi::ControllerParam panorama;

		Haruhi::Param<int> enabled;
	};

	/**
	 * Effect: Waveshaper
	 */
	struct Waveshaper: public Effect
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Waveshaper)

		enum {
			HARUHI_MIKURU_PARAM (Gain,					       0,	+1000000,	 +100000,	 +100000)
			HARUHI_MIKURU_PARAM (Parameter,				       0,	+1000000,	+1000000,	 +500000)
		};

		Haruhi::ControllerParam gain;
		Haruhi::ControllerParam parameter;

		Haruhi::Param<int> type;
	};

	/**
	 * Effect: Reverb
	 */
	struct Reverb: public Effect
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Reverb)

		enum {
			HARUHI_MIKURU_PARAM (RoomSize,				       0,	+1000000,	+1000000,	 +500000)
			HARUHI_MIKURU_PARAM (Width,					       0,	+1000000,	+1000000,	+1000000)
			HARUHI_MIKURU_PARAM (Damp,					       0,	+1000000,	+1000000,	 +500000)
		};

		Haruhi::ControllerParam room_size;
		Haruhi::ControllerParam width;
		Haruhi::ControllerParam damp;

		Haruhi::Param<int> mode;
	};

	/**
	 * Effect: Delay
	 */
	struct Delay: public Effect
	{
		HARUHI_MIKURU_PARAMS_STANDARD_METHODS (Delay)

		enum {
			HARUHI_MIKURU_PARAM (Feedback,				       0,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (CrossFeedback,			       0,	+1000000,	+1000000,	       0)
			HARUHI_MIKURU_PARAM (Level,					       0,	+1000000,	+1000000,	       0)
		};

		Haruhi::ControllerParam feedback_l;
		Haruhi::ControllerParam feedback_r;
		Haruhi::ControllerParam cross_feedback_l;
		Haruhi::ControllerParam cross_feedback_r;
		Haruhi::ControllerParam level_l;
		Haruhi::ControllerParam level_r;

		Haruhi::Param<int> tempo; // 1000 * BPM. 0 means to use Graph BPM
		Haruhi::Param<int> enabled_l;
		Haruhi::Param<int> enabled_r;
		Haruhi::Param<int> note_length_l;
		Haruhi::Param<int> note_length_r;
		Haruhi::Param<int> note_multiplicator_l;
		Haruhi::Param<int> note_multiplicator_r;
		Haruhi::Param<int> note_adjust_l;
		Haruhi::Param<int> note_adjust_r;
	};
};

} // namespace MikuruPrivate

#undef HARUHI_MIKURU_PARAMS_STANDARD_METHODS
#undef HARUHI_MIKURU_PARAM

#endif

