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

// Standard:
#include <cstddef>
#include <algorithm>

// Local:
#include "ramp_smoother.h"


namespace Haruhi {

namespace DSP {

RampSmoother::RampSmoother (unsigned int max_samples):
	_current (0.0f),
	_vector (+0.0f),
	_target (0.0f),
	_samples (0),
	_samples_from_last_setup (0),
	_samples_max (max_samples),
	_first_value_initialized (false)
{
}


void
RampSmoother::set_value (float value)
{
	if (_target != value)
	{
		set_actual_samples (std::min (_samples_from_last_setup, _samples_max));
		if (_first_value_initialized)
		{
			_target = value;
			_vector = (_target - _current) / _samples;
		}
		else
		{
			_target = value;
			_current = value;
			_vector = +0.0;
			_first_value_initialized = true;
		}
		_samples_from_last_setup = 0;
	}
}


void
RampSmoother::reset (float value)
{
	_target = value;
	_vector = +0.0f;
	_current = value;
	_samples_from_last_setup = 0;
}

} // namespace DSP

} // namespace Haruhi

