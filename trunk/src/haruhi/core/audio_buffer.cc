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

// Standard:
#include <algorithm>
#include <functional>
#include <memory.h>

// Haruhi:
#include <haruhi/utility/pool_allocator.h>
#include <haruhi/utility/backtrace.h>

// Local:
#include "audio_buffer.h"


namespace Core {

POOL_ALLOCATOR_FOR (AudioBuffer)

Buffer::TypeID AudioBuffer::TYPE = "Core::AudioBuffer";


AudioBuffer::AudioBuffer (std::size_t size):
	_data (new Sample[size]),
	_size (size)
{
	clear();
}


AudioBuffer::~AudioBuffer()
{
	delete[] _data;
}


void
AudioBuffer::resize (std::size_t size)
{
	if (size != _size)
	{
		delete[] _data;
		_size = size;
		_data = new Sample[_size];
		clear();
	}
}

} // namespace Core

