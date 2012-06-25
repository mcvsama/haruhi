/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__PLUGINS__YUKI__DUAL_FILTER_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__DUAL_FILTER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/filter.h>
#include <haruhi/dsp/one_pole_smoother.h>
#include <haruhi/utility/frequency.h>

// Local:
#include "filter_ir.h"
#include "params.h"


namespace Yuki {

class DualFilter
{
  public:
	typedef DSP::Filter<FilterImpulseResponse::Order, FilterImpulseResponse::ResponseType> FilterType;

	static const int MaxStages = 5;

	enum Configuration
	{
		Serial		= 0,
		Parallel	= 1,
	};

  public:
	DualFilter (Params::Filter* params_1, Params::Filter* params_2);

	/**
	 * Resets all filters to default state.
	 */
	void
	reset();

	/**
	 * \param	configuration (serial/parallel)
	 */
	void
	configure (Configuration configuration, Frequency sample_rate);

	/**
	 * Returns true if actual filtering was done, false otherwise (output buffer was not filled).
	 * All buffers must be distinct.
	 */
	bool
	process (Haruhi::AudioBuffer* input_1, Haruhi::AudioBuffer* input_2,
			 Haruhi::AudioBuffer* buffer_1, Haruhi::AudioBuffer* buffer_2,
			 Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2);

  private:
	/**
	 * All buffers must be distinct.
	 */
	void
	filterout (FilterType* filters, int stages, Haruhi::AudioBuffer* input, Haruhi::AudioBuffer* buffer, Haruhi::AudioBuffer* output) noexcept;

  public:
	Configuration			_configuration;
	Params::Filter*			_params_1;
	Params::Filter*			_params_2;
	FilterImpulseResponse	_impulse_response_1;
	FilterImpulseResponse	_impulse_response_2;
	// Two channels, for each up to 5 stages:
	FilterType				_filter_1[2][5];
	FilterType				_filter_2[2][5];
	// Smoothers:
	DSP::OnePoleSmoother	_smoother_1_frequency;
	DSP::OnePoleSmoother	_smoother_1_resonance;
	DSP::OnePoleSmoother	_smoother_1_gain;
	DSP::OnePoleSmoother	_smoother_1_attenuation;
	DSP::OnePoleSmoother	_smoother_2_frequency;
	DSP::OnePoleSmoother	_smoother_2_resonance;
	DSP::OnePoleSmoother	_smoother_2_gain;
	DSP::OnePoleSmoother	_smoother_2_attenuation;
};

} // namespace Yuki

#endif

