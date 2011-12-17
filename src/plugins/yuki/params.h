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

// Haruhi:
#include <haruhi/dsp/harmonics_wave.h>
#include <haruhi/lib/controller_param.h>
#include <haruhi/lib/param.h>
#include <haruhi/utility/saveable_state.h>


/*
 * Global/external macros.
 */

// Useful for ControllerProxy constructors:
#define HARUHI_YUKI_MINMAX(klass_and_enum)										\
	Params::klass_and_enum##Min, Params::klass_and_enum##Max

// Useful for Knob constructors:
#define HARUHI_YUKI_PARAMS_FOR_KNOB(klass_and_enum)								\
	1.0 * Params::klass_and_enum##Min / Params::klass_and_enum##Denominator,	\
	1.0 * Params::klass_and_enum##Max / Params::klass_and_enum##Denominator

// Useful for Knob constructors:
#define HARUHI_YUKI_PARAMS_FOR_KNOB_WITH_STEPS(klass_and_enum, steps)			\
	1.0 * Params::klass_and_enum##Min / Params::klass_and_enum##Denominator,	\
	1.0 * Params::klass_and_enum##Max / Params::klass_and_enum##Denominator,	\
	(Params::klass_and_enum##Max - Params::klass_and_enum##Min) / steps

/*
 * Local macros, will be undefed.
 */

#define HARUHI_YUKI_PARAMS_STANDARD_METHODS(klass)								\
	typedef klass ThisType;														\
	klass();																	\
	virtual ~klass() { }														\
	protected:																	\
	void get_params (Haruhi::BaseParam const**, size_t max_entries) const;		\
	public:

#define HARUHI_YUKI_PARAM(name, min, max, denominator, deflt)					\
	enum {																		\
		name##Min = min,														\
		name##Max = max,														\
		name##Denominator = denominator,										\
		name##Default = deflt,													\
	};


namespace Yuki {

struct Params
{
	template<class SubClass>
		class SaveableParams: public SaveableState
		{
		  public:
			typedef Haruhi::BaseParam SubClass::* MemberBaseParamPtr;

		  public:
			/**
			 * Sanitize params - call .sanitize() on each param.
			 */
			void
			sanitize();

			/**
			 * Dump/marshal all parameters into XML node.
			 */
			void
			save_state (QDomElement& parent) const;

			/**
			 * Restore parameter values from XML.
			 */
			void
			load_state (QDomElement const& element);

		  protected:
			/**
			 * Get array of params.
			 * \param	tab Here will be stored array of pointers to params.
			 * \param	max_entries Max number of entries to be stored (length of the tab array).
			 */
			virtual void
			get_params (Haruhi::BaseParam const** tab, size_t max_entries) const = 0;
		};

	/**
	 * Main control panel params.
	 */
	struct Main: public SaveableParams<Main>
	{
		HARUHI_YUKI_PARAMS_STANDARD_METHODS (Main)

		HARUHI_YUKI_PARAM (Volume,					       0,	+1000000,	+1000000,	 +938445) // -1.5dB/20*log_10/exp
		HARUHI_YUKI_PARAM (Panorama,				-1000000,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (Detune,					-1000000,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (StereoWidth,				-1000000,	+1000000,	+1000000,	+1000000)

		Haruhi::ControllerParam volume;
		Haruhi::ControllerParam panorama;
		Haruhi::ControllerParam detune;
		Haruhi::ControllerParam stereo_width;

		Haruhi::Param<int> enabled;
		Haruhi::Param<unsigned int> polyphony;

		static const int NUM_PARAMS = 6;
	};

	/**
	 * Filter parameters.
	 */
	struct Filter: public SaveableParams<Filter>
	{
		typedef Haruhi::ControllerParam Filter::* ControllerParamPtr;
		typedef Haruhi::Param<int> Filter::* IntParamPtr;

		HARUHI_YUKI_PARAMS_STANDARD_METHODS (Filter)

		HARUHI_YUKI_PARAM (Frequency,				       0,	+2400000,	 +100000,	 +100000)
		HARUHI_YUKI_PARAM (Resonance,				       0,	+1000000,	 +100000,	 +100000)
		HARUHI_YUKI_PARAM (Gain,					       0,	+2000000,	 +100000,	       0)
		HARUHI_YUKI_PARAM (Attenuation,				       0,	+1000000,	+1000000,	+1000000)

		Haruhi::ControllerParam frequency;
		Haruhi::ControllerParam resonance;
		Haruhi::ControllerParam gain;
		Haruhi::ControllerParam attenuation;

		Haruhi::Param<int> enabled;
		Haruhi::Param<int> type;
		Haruhi::Param<int> stages;
		Haruhi::Param<int> limiter_enabled;

		static const int NUM_PARAMS = 8;
	};

	/**
	 * Voice-specific params.
	 */
	struct Voice: public SaveableParams<Voice>
	{
		typedef Haruhi::ControllerParam Voice::* ControllerParamPtr;

		HARUHI_YUKI_PARAMS_STANDARD_METHODS (Voice)

		HARUHI_YUKI_PARAM (Amplitude,				       0,	+1000000,	+1000000,	+1000000)
		HARUHI_YUKI_PARAM (Frequency,				-1000000,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (Panorama,				-1000000,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (Detune,					-1000000,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (Pitchbend,				-1000000,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (VelocitySens,			-1000000,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (UnisonIndex,				      +1,	     +10,	      +1,	      +1)
		HARUHI_YUKI_PARAM (UnisonSpread,			       0,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (UnisonInit,				-1000000,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (UnisonNoise,				       0,	+1000000,	+1000000,	       0)

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

		static const int NUM_PARAMS = 22;

		// Embedded dual Filter params:
		Filter filter[2];
	};

	/**
	 * General Part-specific params.
	 */
	struct Part: public SaveableParams<Part>
	{
		HARUHI_YUKI_PARAMS_STANDARD_METHODS (Part)

		HARUHI_YUKI_PARAM (Volume,					       0,	+1000000,	+1000000,	 +938445) // -1.5dB/20*log_10/exp
		HARUHI_YUKI_PARAM (PortamentoTime,			       0,	+1000000,	 +100000,	       0)
		HARUHI_YUKI_PARAM (Phase,					-1000000,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (NoiseLevel,				       0,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (WaveShape,				       0,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (ModulatorAmplitude,		       0,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (ModulatorIndex,			      +1,	     +32,	      +1,	      +1)
		HARUHI_YUKI_PARAM (ModulatorShape,			       0,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (Harmonic,				-1000000,	+1000000,	+1000000,	       0)
		HARUHI_YUKI_PARAM (HarmonicPhase,			-1000000,	+1000000,	+1000000,	       0)

		static const unsigned int HarmonicsNumber = Haruhi::DSP::HarmonicsWave::HarmonicsNumber;

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

		Haruhi::Param<int> part_enabled;
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

		static const int NUM_PARAMS = 22 + HarmonicsNumber + HarmonicsNumber;

		// Embedded Voice params template (also includes Filter params):
		Voice voice;
	};
};


template<class SubClass>
	inline void
	Params::SaveableParams<SubClass>::sanitize()
	{
		Haruhi::BaseParam const** params = reinterpret_cast<Haruhi::BaseParam const**> (alloca (sizeof (Haruhi::BaseParam*) * SubClass::NUM_PARAMS));
		get_params (params, SubClass::NUM_PARAMS);
		for (int i = 0; i < SubClass::NUM_PARAMS; ++i)
			params[i]->sanitize();
	}


template<class SubClass>
	inline void
	Params::SaveableParams<SubClass>::save_state (QDomElement& parent) const
	{
		Haruhi::BaseParam const** params = reinterpret_cast<Haruhi::BaseParam const**> (alloca (sizeof (Haruhi::BaseParam*) * SubClass::NUM_PARAMS));
		get_params (params, SubClass::NUM_PARAMS);
		for (int i = 0; i < SubClass::NUM_PARAMS; ++i)
		{
			// TODO Save knob settings in UI widget's save_state().
			// TODO Or make it possible to access curve params from Param<>.
			// TODO Or just move curve/user-limits to Param<>.
			QDomElement param_el = parent.ownerDocument().createElement ("parameter");
			param_el.setAttribute ("name", params[i]->name());
			params[i]->save_state (param_el);
			parent.appendChild (param_el);
		}
	}


template<class SubClass>
	inline void
	Params::SaveableParams<SubClass>::load_state (QDomElement const& /*element*/)
	{
		// TODO
	}

} // namespace Yuki

#undef HARUHI_YUKI_PARAMS_STANDARD_METHODS
#undef HARUHI_YUKI_PARAM

#endif

