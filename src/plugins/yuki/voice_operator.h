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

#ifndef HARUHI__PLUGINS__YUKI__OPERATOR_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__OPERATOR_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/functions.h>
#include <haruhi/dsp/wavetable.h>


namespace Yuki {

using Haruhi::Sample;

/**
 * Modulator oscillator.
 */
class VoiceOperator
{
  public:
	VoiceOperator();

	/**
	 * Set frequency source buffer.
	 */
	void
	set_frequency_source (Haruhi::AudioBuffer* source);

	/**
	 * Set amplitude source buffer.
	 */
	void
	set_amplitude_source (Haruhi::AudioBuffer* source);

	/**
	 * Set operator detune.
	 */
	void
	set_detune (Sample detune);

	/**
	 * Fill output buffer.
	 */
	void
	fill (Haruhi::AudioBuffer* output);

  private:
	Haruhi::AudioBuffer*	_frequency_source;
	Haruhi::AudioBuffer*	_amplitude_source;
	Sample					_detune;
	Sample					_phase;
};


inline void
VoiceOperator::set_frequency_source (Haruhi::AudioBuffer* source)
{
	_frequency_source = source;
}


inline void
VoiceOperator::set_amplitude_source (Haruhi::AudioBuffer* source)
{
	_amplitude_source = source;
}


inline void
VoiceOperator::set_detune (Sample detune)
{
	_detune = detune;
}


inline void
VoiceOperator::fill (Haruhi::AudioBuffer* output)
{
	assert (output != 0);

	Sample* const fs = _frequency_source->begin();
	Sample f;

	// Oscillate:
	for (std::size_t i = 0; i < output->size(); ++i)
	{
		f = bound (fs[i] * _detune, 0.0f, 0.5f);
		_phase = mod1 (_phase + f);
		// To keep β constant the output amplitude must be scaled depending on frequency:
		(*output)[i] = f * Haruhi::DSP::base_sin<5, Haruhi::Sample> (_phase * 2.0f - 1.0f);
	}

	// Amplitude modulation:
	output->attenuate (_amplitude_source);
}

} // namespace Yuki

#endif

