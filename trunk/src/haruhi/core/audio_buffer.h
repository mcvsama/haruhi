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

#ifndef HARUHI__CORE__AUDIO_BUFFER_H__INCLUDED
#define HARUHI__CORE__AUDIO_BUFFER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/utility/pool_allocator.h>

// Local:
#include "audio.h"
#include "buffer.h"
#include "exception.h"


namespace Core {

class AudioBuffer: public Core::Buffer
{
	USES_POOL_ALLOCATOR (AudioBuffer)

  public:
	static TypeID TYPE;

  public:
	AudioBuffer (std::size_t size);

	~AudioBuffer();

	TypeID
	type() const { return AudioBuffer::TYPE; }

	void
	clear() { std::fill (begin(), end(), 0.0); }

	/**
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	fill (Buffer* other)
	{
		if (other->type() != AudioBuffer::TYPE)
			throw Exception ("incompatible buffers");
		AudioBuffer* buf = static_cast<AudioBuffer*> (other);
		std::copy (buf->begin(), buf->end(), begin());
	}

	/**
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	mixin (Buffer* other)
	{
		if (other->type() != AudioBuffer::TYPE)
			throw Exception ("incompatible buffers");
		AudioBuffer* buf = static_cast<AudioBuffer*> (other);
		std::transform (buf->begin(), buf->end(), begin(), begin(), std::plus<Sample>());
	}

	void
	resize (std::size_t size);

	std::size_t
	size() const
	{
		return _size;
	}

	Sample*
	begin() const
	{
		return _data;
	}

	Sample*
	end() const
	{
		return _data + _size;
	}

	Sample&
	operator[] (std::size_t const i)
	{
		return _data[i];
	}

	Sample const&
	operator[] (std::size_t const i) const
	{
		return _data[i];
	}

  public:
	Sample*		_data;
	std::size_t	_size;
};

} // namespace Core

#endif

