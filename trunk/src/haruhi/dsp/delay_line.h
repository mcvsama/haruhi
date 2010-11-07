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

#ifndef HARUHI__DSP__DELAY_LINE_H__INCLUDED
#define HARUHI__DSP__DELAY_LINE_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/core/audio.h>


namespace Haruhi {

namespace DSP {

class DelayLine
{
  public:
	/**
	 * \param	buffer_size is number of samples write/read each time.
	 * \param	delay is number of samples to delay.
	 * \param	size is number of samples read/written each time.
	 */
	DelayLine (std::size_t delay, std::size_t max_delay, std::size_t size);

	/**
	 * Should be called only after read and before next write.
	 * \param	delay is number of samples to delay.
	 */
	void
	set_delay (std::size_t delay);

	/**
	 * Sets maximum delay in samples.
	 */
	void
	set_max_delay (std::size_t max_delay);

	/**
	 * Should be called only after read and before next write.
	 * Sets number of samples read/written with write/read methods.
	 */
	void
	set_size (std::size_t size);

	/**
	 * Writes buffer-size samples to delay line.
	 * Set number of samples with set_size();
	 * \param	data is pointer to source buffer.
	 */
	void
	write (Core::Sample const* data);

	/**
	 * Reads delayed buffer-size samples from delay line.
	 * Subsequent calls between writings always will return the same result.
	 * \param	data is pointer to output buffer.
	 */
	void
	read (Core::Sample* data);

	/**
	 * Clears data buffer.
	 */
	void
	clear();

  private:
	Core::Sample*	_data;
	std::size_t		_max_delay;		// Whole buffer size (number of samples).
	std::size_t		_size;			// Number of samples read/written on each round.
	std::size_t		_delay;
	std::size_t		_wpos;
};

} // namespace DSP

} // namespace Haruhi

#endif

