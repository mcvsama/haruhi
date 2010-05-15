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

#ifndef HARUHI__UNITS__MIKURU__VOICE_COMMONS_H__INCLUDED
#define HARUHI__UNITS__MIKURU__VOICE_COMMONS_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/core/audio_buffer.h>


class Mikuru;


namespace MikuruPrivate {

class VoiceCommons
{
  public:
	VoiceCommons (std::size_t buffers_size);

	void
	resize_buffers (std::size_t buffers_size);

  public:
	// Buffers:
	Core::AudioBuffer	temp1;
	Core::AudioBuffer	frequency_buffer;
	Core::AudioBuffer	amplitude_buffer;
	Core::AudioBuffer	oscillator_buffer;
	Core::AudioBuffer	filter_buffer1;
	Core::AudioBuffer	filter_buffer2;
	Core::AudioBuffer	output_buffer;
	Core::AudioBuffer	output_buffer1;
	Core::AudioBuffer	output_buffer2;
};

} // namespace MikuruPrivate

#endif

