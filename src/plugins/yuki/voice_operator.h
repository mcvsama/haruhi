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
	 * Fill output buffers.
	 * \param	output Fully modulated output (amplitude and frequency modulated).
	 * \param	fm_output Frequency modulated output. Amplitude modulation does
	 *			not affect this buffer, hence it's always at 0 dB level.
	 */
	void
	fill (Haruhi::AudioBuffer* output, Haruhi::AudioBuffer* fm_output) noexcept;

  private:
	Haruhi::AudioBuffer*	_frequency_source	= nullptr;
	Haruhi::AudioBuffer*	_amplitude_source	= nullptr;
	Sample					_detune				= 0.0f;
	Sample					_phase				= 0.0f;
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

} // namespace Yuki

#endif

