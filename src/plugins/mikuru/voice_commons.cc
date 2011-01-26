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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "voice_commons.h"


namespace MikuruPrivate {

VoiceCommons::VoiceCommons (std::size_t buffers_size):
	temp1 (buffers_size),
	frequency_buffer (buffers_size),
	amplitude_buffer (buffers_size),
	oscillator_buffer (buffers_size),
	filter_buffer1 (buffers_size),
	filter_buffer2 (buffers_size),
	output_buffer (buffers_size),
	output_buffer1 (buffers_size),
	output_buffer2 (buffers_size)
{
}


void
VoiceCommons::resize_buffers (std::size_t buffers_size)
{
	temp1.resize (buffers_size);
	frequency_buffer.resize (buffers_size);
	amplitude_buffer.resize (buffers_size);
	oscillator_buffer.resize (buffers_size);
	filter_buffer1.resize (buffers_size);
	filter_buffer2.resize (buffers_size);
	output_buffer.resize (buffers_size);
	output_buffer1.resize (buffers_size);
	output_buffer2.resize (buffers_size);
}

} // namespace MikuruPrivate

