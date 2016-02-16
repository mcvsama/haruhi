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

#ifndef HARUHI__GRAPH__AUDIO_BUFFER_H__INCLUDED
#define HARUHI__GRAPH__AUDIO_BUFFER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdlib>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/pool_allocator.h>
#include <haruhi/utility/simd_ops.h>


namespace Haruhi {

class AudioBuffer
{
	USES_POOL_ALLOCATOR (AudioBuffer)

  public:
	typedef std::size_t		size_type;
	typedef std::ptrdiff_t	difference_type;

  public:
	AudioBuffer (std::size_t size = 0);

	~AudioBuffer() noexcept;

	/**
	 * Clears (zeroes) buffer.
	 */
	void
	clear() noexcept;

	/**
	 * Fills this buffer from other buffer.
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	fill (AudioBuffer const* other) noexcept;

	/**
	 * Fills this buffer with given scalar.
	 */
	void
	fill (Sample value) noexcept;

	/**
	 * Calls add().
	 */
	void
	mixin (AudioBuffer const* other) noexcept;

	/**
	 * Calls add().
	 */
	void
	mixin (AudioBuffer const* other, Sample attenuate_other) noexcept;

	/**
	 * Calls add().
	 */
	void
	mixin (AudioBuffer const* other, AudioBuffer const* attenuation_buffer) noexcept;

	/**
	 * Adds (mixes) other buffer to this.
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	add (AudioBuffer const* other) noexcept;

	/**
	 * Adds (mixes) attenuated other buffer to this.
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	add (AudioBuffer const* other, Sample attenuate_other) noexcept;

	/**
	 * Add (mix) other buffer attenuated by attenuation_buffer, to this buffer.
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	add (AudioBuffer const* other, AudioBuffer const* attenuation_buffer) noexcept;

	/**
	 * Substracts other buffer from this.
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	sub (AudioBuffer const* other) noexcept;

	/**
	 * Attenuates this buffer by other buffer.
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	attenuate (AudioBuffer const* other) noexcept;

	/**
	 * Attenuates this buffer by scalar.
	 */
	void
	attenuate (Sample value) noexcept;

	/**
	 * Attenuates this buffer by other buffer and by scalar.
	 * Other buffer must be static_castable to AudioBuffer.
	 */
	void
	attenuate (AudioBuffer const* other, Sample value) noexcept;

	/**
	 * Negates this buffer.
	 */
	void
	negate() noexcept;

	/**
	 * Resize buffer to given number of samples.
	 */
	void
	resize (std::size_t size);

	/**
	 * Return buffer size (in samples).
	 */
	std::size_t
	size() const noexcept;

	/**
	 * Return pointer to the buffer.
	 */
	Sample*
	begin() const noexcept;

	/**
	 * Return pointer to past-the end sample in buffer.
	 */
	Sample*
	end() const noexcept;

	/**
	 * Shorthand methods for accessing samples in buffer.
	 */
	Sample&
	operator[] (std::size_t const i) noexcept;

	Sample const&
	operator[] (std::size_t const i) const noexcept;

  public:
	/**
	 * Allocates buffer for given number of samples.
	 * AudioBuffer is aligned to 32-byte boundary so SIMD
	 * instructions can be used on it.
	 */
	static Sample*
	allocate (std::size_t samples);

	/**
	 * Deallocates memory allocated with allocate().
	 */
	static void
	deallocate (Sample* buffer) noexcept;

  public:
	Sample*		_data;
	std::size_t	_size;
	Sample*		_end;
};


inline void
AudioBuffer::clear() noexcept
{
	SIMD::clear_buffer (begin(), size());
}


inline void
AudioBuffer::fill (AudioBuffer const* other) noexcept
{
	assert (begin() != nullptr);
	assert (other->begin() != nullptr);
	assert (other->size() == size());
	SIMD::copy_buffer (begin(), other->begin(), size());
}


inline void
AudioBuffer::fill (Sample value) noexcept
{
	SIMD::fill_buffer (begin(), size(), value);
}


inline void
AudioBuffer::mixin (AudioBuffer const* other) noexcept
{
	add (other);
}


inline void
AudioBuffer::mixin (AudioBuffer const* other, Sample attenuate_other) noexcept
{
	add (other, attenuate_other);
}


inline void
AudioBuffer::mixin (AudioBuffer const* other, AudioBuffer const* attenuation_buffer) noexcept
{
	add (other, attenuation_buffer);
}


inline void
AudioBuffer::add (AudioBuffer const* other) noexcept
{
	assert (begin() != nullptr);
	assert (other->begin() != nullptr);
	assert (other->size() == size());
	SIMD::add_buffers (begin(), other->begin(), size());
}


inline void
AudioBuffer::add (AudioBuffer const* other, Sample attenuate_other) noexcept
{
	assert (begin() != nullptr);
	assert (other->begin() != nullptr);
	assert (other->size() == size());
	SIMD::add_buffers (begin(), other->begin(), attenuate_other, size());
}


inline void
AudioBuffer::add (AudioBuffer const* other, AudioBuffer const* attenuation_buffer) noexcept
{
	assert (begin() != nullptr);
	assert (other->begin() != nullptr);
	assert (other->size() == size());
	assert (attenuation_buffer->begin() != nullptr);
	assert (attenuation_buffer->size() == size());

	for (std::size_t i = 0; i < size(); ++i)
		(*this)[i] += (*other)[i] * (*attenuation_buffer)[i];
}


inline void
AudioBuffer::sub (AudioBuffer const* other) noexcept
{
	assert (begin() != nullptr);
	assert (other->begin() != nullptr);
	assert (other->size() == size());
	SIMD::sub_buffers (begin(), other->begin(), size());
}


inline void
AudioBuffer::attenuate (AudioBuffer const* other) noexcept
{
	assert (begin() != nullptr);
	assert (other->begin() != nullptr);
	assert (other->size() == size());
	SIMD::multiply_buffers (begin(), other->begin(), size());
}


inline void
AudioBuffer::attenuate (Sample value) noexcept
{
	SIMD::multiply_buffer_by_scalar (begin(), size(), value);
}


inline void
AudioBuffer::attenuate (AudioBuffer const* other, Sample value) noexcept
{
	assert (begin() != nullptr);
	assert (other->begin() != nullptr);
	assert (other->size() == size());
	SIMD::multiply_buffers_and_by_scalar (begin(), other->begin(), size(), value);
}


inline void
AudioBuffer::negate() noexcept
{
	SIMD::negate_buffer (begin(), size());
}


inline std::size_t
AudioBuffer::size() const noexcept
{
	return _size;
}


inline Sample*
AudioBuffer::begin() const noexcept
{
	return _data;
}


inline Sample*
AudioBuffer::end() const noexcept
{
	return _end;
}


inline Sample&
AudioBuffer::operator[] (std::size_t const i) noexcept
{
	return _data[i];
}


inline Sample const&
AudioBuffer::operator[] (std::size_t const i) const noexcept
{
	return _data[i];
}


inline Sample*
AudioBuffer::allocate (std::size_t samples)
{
	if (samples == 0)
		return nullptr;
	void* ret;
	if (posix_memalign (&ret, 32, sizeof (Sample) * samples) != 0)
		return nullptr;
	return static_cast<Sample*> (ret);
}


inline void
AudioBuffer::deallocate (Sample* buffer) noexcept
{
	free (buffer);
}

} // namespace Haruhi

#endif

