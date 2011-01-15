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

#ifndef HARUHI__UNITS__MIKURU__DOUBLE_FILTER_H__INCLUDED
#define HARUHI__UNITS__MIKURU__DOUBLE_FILTER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/filter.h>
#include <haruhi/dsp/one_pole_smoother.h>

// Local:
#include "rbj_impulse_response.h"
#include "filter.h"
#include "params.h"


class Mikuru;

namespace MikuruPrivate {

class DoubleFilter
{
  public:
	enum {
		NumFilters = 5,
	};

	enum Configuration
	{
		ConfigurationSerial		= 0,
		ConfigurationParallel	= 1,
	};

  public:
	DoubleFilter (Mikuru* mikuru);

	void
	reset();

	/**
	 * \param	configuration (serial/parallel)
	 */
	void
	configure (Configuration configuration, Params::Filter* params1, Params::Filter* param2);

	/**
	 * Returns true if actual filtering was done, false otherwise (output buffer was not filled).
	 * All buffers must be distinct.
	 */
	bool
	process (Haruhi::AudioBuffer& input, Haruhi::AudioBuffer& buffer1, Haruhi::AudioBuffer& buffer2, Haruhi::AudioBuffer& output);

  private:
	/**
	 * All buffers must be distinct.
	 */
	void
	filterout (DSP::Filter* filters, int passes, Haruhi::AudioBuffer& input, Haruhi::AudioBuffer& buffer, Haruhi::AudioBuffer& output);

  public:
	Mikuru*						_mikuru;
	Configuration				_configuration;
	Params::Filter				_params1;
	Params::Filter				_params2;

	RBJImpulseResponse			_impulse_response1;
	RBJImpulseResponse			_impulse_response2;

	DSP::Filter					_filter1[5];
	DSP::Filter					_filter2[5];

	DSP::OnePoleSmoother		_smoother_filter1_frequency;
	DSP::OnePoleSmoother		_smoother_filter1_resonance;
	DSP::OnePoleSmoother		_smoother_filter1_gain;
	DSP::OnePoleSmoother		_smoother_filter1_attenuation;
	DSP::OnePoleSmoother		_smoother_filter2_frequency;
	DSP::OnePoleSmoother		_smoother_filter2_resonance;
	DSP::OnePoleSmoother		_smoother_filter2_gain;
	DSP::OnePoleSmoother		_smoother_filter2_attenuation;
};

} // namespace MikuruPrivate

#endif

