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
#include <cstddef>
#include <algorithm>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "delay_line.h"


namespace Haruhi {

namespace DSP {

DelayLine::DelayLine (std::size_t delay, std::size_t max_delay, std::size_t size):
	_data (0),
	_max_delay (max_delay),
	_size (size),
	_delay (delay),
	_wpos (0)
{
	assert (size > 0);
	assert (max_delay > 0);
	assert (max_delay > size);

	set_max_delay (max_delay);
	set_delay (delay);
	clear();
}


void
DelayLine::set_delay (std::size_t delay)
{
	assert (delay < _max_delay);

	_delay = delay;
}


void
DelayLine::set_max_delay (std::size_t max_delay)
{
	assert (max_delay > 0);

	delete _data;
	_data = new Sample[sizeof (Sample) * max_delay];
	_max_delay = max_delay;

	if (_delay > _max_delay)
		_delay = _max_delay;

	clear();
}


void
DelayLine::write (Sample const* data)
{
	if (_wpos + _size < _max_delay)
	{
		std::copy (data, data + _size, _data + _wpos);
		_wpos = (_wpos + _size) % _max_delay;
	}
	else
	{
		const std::size_t n = _wpos + _size - _max_delay;
		std::copy (data, data + n, _data + _wpos);
		std::copy (data + n, data + _size, _data);
		_wpos = _size - n;
	}
}


void
DelayLine::read (Sample* data)
{
	// Delayed pos:
	const std::size_t pos = _wpos > _delay
		? _wpos - _delay
		: _max_delay - _delay + _wpos;

	if (pos < _size)
	{
		const std::size_t n = _size - pos;
		std::copy (_data + _max_delay - n, _data + _max_delay, data);
		std::copy (_data, _data + pos, data + n);
	}
	else
	{
		std::copy (_data + pos - _size, _data + pos, data);
	}
}


void
DelayLine::clear()
{
	std::fill (_data, _data + _max_delay, 0.0f);
}

} // namespace DSP

} // namespace Haruhi

