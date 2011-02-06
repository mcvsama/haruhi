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

#ifndef HARUHI__GRAPH__AUDIO_BUFFER_H__INCLUDED
#define HARUHI__GRAPH__AUDIO_BUFFER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdlib>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/pool_allocator.h>
#include <haruhi/utility/simd_ops.h>

// Local:
#include "buffer.h"
#include "exception.h"


namespace Haruhi {

class AudioBuffer: public Buffer
{
	USES_POOL_ALLOCATOR (AudioBuffer)

  public:
	static TypeID TYPE;

  public:
	AudioBuffer (std::size_t size = 0);

	~AudioBuffer();

	TypeID
	type() const { return AudioBuffer::TYPE; }

	/**
	 * Clears (zeroes) buffer.
	 */
	void
	clear() { SIMD::clear_buffer (begin(), size()); }

	/**
	 * Fills this buffer from other buffer.
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	fill (Buffer const* other)
	{
		assert (other->type() == AudioBuffer::TYPE);
		AudioBuffer const* buf = static_cast<AudioBuffer const*> (other);
		assert (buf->size() == size());
		SIMD::copy_buffer (begin(), buf->begin(), size());
	}

	/**
	 * Calls add().
	 */
	void
	mixin (Buffer const* other)
	{
		add (other);
	}

	/**
	 * Adds (mixes) other buffer to this.
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	add (Buffer const* other)
	{
		assert (other->type() == AudioBuffer::TYPE);
		AudioBuffer const* buf = static_cast<AudioBuffer const*> (other);
		assert (buf->size() == size());
		SIMD::add_buffers (begin(), buf->begin(), size());
	}

	/**
	 * Substracts other buffer from this.
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	sub (Buffer const* other)
	{
		assert (other->type() == AudioBuffer::TYPE);
		AudioBuffer const* buf = static_cast<AudioBuffer const*> (other);
		assert (buf->size() == size());
		SIMD::sub_buffers (begin(), buf->begin(), size());
	}

	/**
	 * Attenuates this buffer by other buffer.
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	attenuate (Buffer const* other)
	{
		assert (other->type() == AudioBuffer::TYPE);
		AudioBuffer const* buf = static_cast<AudioBuffer const*> (other);
		assert (buf->size() == size());
		SIMD::multiply_buffers (begin(), buf->begin(), size());
	}

	/**
	 * Attenuates this buffer by scalar.
	 */
	void
	attenuate (Sample value)
	{
		SIMD::multiply_buffer_by_scalar (begin(), size(), value);
	}

	/**
	 * Negates this buffer.
	 */
	void
	negate()
	{
		SIMD::negate_buffer (begin(), size());
	}

	void
	resize (std::size_t size);

	std::size_t
	size() const { return _size; }

	Sample*
	begin() const { return _data; }

	Sample*
	end() const { return _end; }

	Sample&
	operator[] (std::size_t const i) { return _data[i]; }

	Sample const&
	operator[] (std::size_t const i) const { return _data[i]; }

  public:
	/**
	 * Allocates buffer for given number of samples.
	 * Buffer is aligned to 32-byte boundary so SIMD
	 * instructions can be used on it.
	 */
	static Sample*
	allocate (std::size_t samples)
	{
		if (samples == 0)
			return 0;
		void* ret;
		if (posix_memalign (&ret, 32, sizeof (Sample) * samples) != 0)
			return 0;
		return static_cast<Sample*> (ret);
	}

	/**
	 * Deallocates memory allocated with allocate().
	 */
	static void
	deallocate (Sample* buffer)
	{
		free (buffer);
	}

  public:
	Sample*		_data;
	std::size_t	_size;
	Sample*		_end;
};

} // namespace Haruhi

#endif

