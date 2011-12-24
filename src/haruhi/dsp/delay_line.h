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

#ifndef HARUHI__DSP__DELAY_LINE_H__INCLUDED
#define HARUHI__DSP__DELAY_LINE_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

namespace DSP {

class DelayLine
{
  public:
	/**
	 * Creates delay line with some default (probably non-usable) parameters.
	 * Call set_* functions to override them.
	 */
	DelayLine();

	/**
	 * \param	delay is the delay length measured in samples number.
	 * \param	max_delay is maximum ever used delay measured in samples number.
	 *          Tells how big buffer should be preallocated.
	 * \param	size is number of samples read/written each time.
	 * 			MUST be >= than max_delay.
	 */
	DelayLine (std::size_t delay, std::size_t max_delay, std::size_t size);

	/**
	 * Returns current delay in samples.
	 */
	std::size_t
	delay() const;

	/**
	 * Should be called only after read and before next write.
	 * \param	delay is number of samples to delay.
	 */
	void
	set_delay (std::size_t delay);

	/**
	 * Returns max possible delay (buffer size) in samples.
	 */
	std::size_t
	max_delay() const;

	/**
	 * Sets maximum delay in samples.
	 * Max delay MUST be >= size set with set_size().
	 */
	void
	set_max_delay (std::size_t max_delay);

	/**
	 * Returns number of samples read/written by read() and write() functions.
	 */
	std::size_t
	size() const;

	/**
	 * Should be called only after read and before next write.
	 * Sets number of samples read/written with write/read methods.
	 * Max delay MUST be >= size.
	 */
	void
	set_size (std::size_t size);

	/**
	 * Writes samples to delay line.
	 * Set number of samples with set_size();
	 * \param	data is pointer to source buffer.
	 */
	void
	write (Sample const* data);

	/**
	 * Reads delayed samples from delay line.
	 * Subsequent calls between writings always will return the same result.
	 * \param	data is pointer to output buffer.
	 */
	void
	read (Sample* data);

	/**
	 * Clears data buffer.
	 */
	void
	clear();

  private:
	Sample*		_data;
	std::size_t	_max_delay;		// Whole buffer size (number of samples).
	std::size_t	_size;			// Number of samples read/written on each round.
	std::size_t	_delay;
	std::size_t	_wpos;
};


inline std::size_t
DelayLine::delay() const
{
	return _delay;
}


inline std::size_t
DelayLine::max_delay() const
{
	return _max_delay;
}


inline std::size_t
DelayLine::size() const
{
	return _size;
}

} // namespace DSP

} // namespace Haruhi

#endif

