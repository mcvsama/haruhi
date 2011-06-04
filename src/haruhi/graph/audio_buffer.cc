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
#include <algorithm>
#include <functional>
#include <memory.h>

// Haruhi:
#include <haruhi/utility/pool_allocator.h>
#include <haruhi/utility/backtrace.h>

// Local:
#include "audio_buffer.h"


namespace Haruhi {

POOL_ALLOCATOR_FOR (AudioBuffer)

Buffer::TypeID AudioBuffer::TYPE = "Haruhi::AudioBuffer";


AudioBuffer::AudioBuffer (std::size_t samples):
	Buffer (AudioBuffer::TYPE),
	// Allocate aligned memory for SIMD instructions:
	_data (allocate (samples)),
	_size (samples),
	_end (_data + _size)
{
	clear();
}


AudioBuffer::~AudioBuffer()
{
	deallocate (_data);
}


void
AudioBuffer::resize (std::size_t samples)
{
	if (_size != samples)
	{
		deallocate (_data);
		_size = samples;
		_data = allocate (_size);
		_end = _data + _size;
		clear();
	}
}

} // namespace Haruhi

