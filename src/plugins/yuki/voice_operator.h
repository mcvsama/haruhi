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
	set_frequency_source (Haruhi::AudioBuffer* source) noexcept;

	/**
	 * Set amplitude source buffer.
	 */
	void
	set_amplitude_source (Haruhi::AudioBuffer* source) noexcept;

	/**
	 * Set operator detune.
	 */
	void
	set_detune (Sample detune) noexcept;

	/**
	 * Fill output buffer.
	 */
	void
	fill (Haruhi::AudioBuffer* output) noexcept;

  private:
	Haruhi::AudioBuffer*	_frequency_source;
	Haruhi::AudioBuffer*	_amplitude_source;
	Sample					_detune;
	Sample					_phase;
};


inline void
VoiceOperator::set_frequency_source (Haruhi::AudioBuffer* source) noexcept
{
	_frequency_source = source;
}


inline void
VoiceOperator::set_amplitude_source (Haruhi::AudioBuffer* source) noexcept
{
	_amplitude_source = source;
}


inline void
VoiceOperator::set_detune (Sample detune) noexcept
{
	_detune = detune;
}


inline void
VoiceOperator::fill (Haruhi::AudioBuffer* output) noexcept
{
	assert (output != 0);

	Sample* const fs = _frequency_source->begin();
	Sample f;
	Sample p = _phase;

	// Oscillate:
	for (std::size_t i = 0; i < output->size(); ++i)
	{
		f = bound (fs[i] * _detune, 0.0f, 0.5f);
		p = mod1 (p + f);
		(*output)[i] = Haruhi::DSP::base_sin<5, Haruhi::Sample> (p * 2.0f - 1.0f);
	}

	_phase = p;

	// Amplitude modulation:
//	output->attenuate (_amplitude_source);
}

} // namespace Yuki

#endif

