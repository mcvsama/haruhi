/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__UTILITY__TIMING_H__INCLUDED
#define HARUHI__UTILITY__TIMING_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdint.h>

// System:
#include <sys/time.h>

// Haruhi:
#include <haruhi/config/all.h>


class Timing
{
  public:
	typedef uint64_t Timestamp;

  public:
	Timing() noexcept
	{
		::gettimeofday (&_timeval, &_timezone);
		_start_microseconds = _timeval.tv_sec * 1000000 + _timeval.tv_usec;
	}

	Timestamp
	microseconds() noexcept
	{
		::gettimeofday (&_timeval, &_timezone);
		return _timeval.tv_sec * 1000000 + _timeval.tv_usec - _start_microseconds;
	}

  private:
	struct timeval	_timeval;
	struct timezone	_timezone;
	Timestamp		_start_microseconds;
};

#endif

