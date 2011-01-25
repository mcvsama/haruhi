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
#include <complex>
#include <cmath>

// Local:
#include "filter.h"


namespace Haruhi {

namespace DSP {

Filter::Filter():
	_impulse_response (0),
	_last_serial (0),
	_a (0),
	_b (0),
	_size (0),
	_px (0),
	_py (0)
{
}


Filter::Filter (ImpulseResponse* impulse_response):
	_impulse_response (0),
	_last_serial (0),
	_a (0),
	_b (0),
	_size (0),
	_px (0),
	_py (0)
{
	assign_impulse_response (impulse_response);
}


Filter::~Filter()
{
	delete[] _px;
	delete[] _py;
}


void
Filter::reset()
{
	std::fill (_px, _px + _size, 0.0f);
	std::fill (_py, _py + _size, 0.0f);
}


void
Filter::assign_impulse_response (ImpulseResponse* impulse_response)
{
	delete[] _px;
	delete[] _py;

	_impulse_response = impulse_response;

	// If ImpulseResponse has been actually assigned (not 0):
	if (_impulse_response)
	{
		_a = _impulse_response->a();
		_b = _impulse_response->b();
		_size = _impulse_response->size();
		_px = new Sample[_size];
		_py = new Sample[_size];

		std::fill (_px, _px + _size, 0.0);
		std::fill (_py, _py + _size, 0.0);
	}
	else
	{
		_a = _b = _px = _py = 0;
		_size = 0;
	}
}

} // namespace DSP

} // namespace Haruhi

