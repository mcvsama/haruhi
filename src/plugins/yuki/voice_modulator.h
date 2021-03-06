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

#ifndef HARUHI__PLUGINS__YUKI__VOICE_MODULATOR_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__VOICE_MODULATOR_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/wavetable.h>

// Local:
#include "params.h"
#include "voice_operator.h"


namespace Yuki {

/**
 * Takes input frequency and amplitude parameters. Creates output amplitude and frequency buffers
 * used as modulation source for VoiceOscillator.
 */
class VoiceModulator
{
  public:
	VoiceModulator (Params::Part* part_params, std::size_t buffer_size);

	/**
	 * Modulate given amplitude and frequency buffers.
	 * Modifies buffers in-place.
	 *
	 * \param	tmp_bufs should be array of at least 2 * Params::Part::OperatorsNumber temporary buffers.
	 */
	void
	modulate (Haruhi::AudioBuffer* amplitude_buf_source, Haruhi::AudioBuffer* frequency_buf_source,
			  Haruhi::AudioBuffer* frequency_buf_target, Haruhi::AudioBuffer* tmp_bufs) noexcept;

	/**
	 * Update buffers sizes.
	 */
	void
	graph_updated (std::size_t buffer_size);

	/**
	 * Set oversampling factor.
	 * Needs Graph lock.
	 */
	void
	set_oversampling (unsigned int oversampling);

  private:
	/**
	 * Update buffers sizes according to Graph params and oversampling.
	 */
	void
	resize_buffers();

	/**
	 * \param	source Buffer with values in range [-1.0, 1.0].
	 */
	void
	modulate_frequency (Haruhi::AudioBuffer* target, Haruhi::AudioBuffer* source, Haruhi::Sample factor) noexcept;

	/**
	 * \param	source Buffer with values in range [-1.0, 1.0].
	 */
	void
	modulate_amplitude (Haruhi::AudioBuffer* target, Haruhi::AudioBuffer* source, Haruhi::Sample factor) noexcept;

  private:
	Params::Part*		_part_params;
	Haruhi::AudioBuffer	_operator_output[Params::Part::OperatorsNumber];
	Haruhi::AudioBuffer	_operator_fm_output[Params::Part::OperatorsNumber];
	VoiceOperator		_operator[Params::Part::OperatorsNumber];
	std::size_t			_buffer_size	= 0;
	unsigned int		_oversampling	= 1;
};

} // namespace Yuki

#endif

