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

// Haruhi:
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "controller_param.h"


namespace Haruhi {

ControllerParam::ControllerParam():
	_minimum (0),
	_maximum (0),
	_denominator (1),
	_default_value (0),
	_smoothing_enabled (false),
	// No need to use atomic during construction:
	_parameter (_minimum),
	_smoothing_parameter (0)
{ }


ControllerParam::ControllerParam (int minimum, int maximum, int denominator, int default_value):
	_minimum (minimum),
	_maximum (maximum),
	_denominator (denominator),
	_default_value (default_value),
	_smoothing_enabled (false),
	// No need to use atomic during construction:
	_parameter (default_value),
	_smoothing_parameter (0)
{ }


ControllerParam&
ControllerParam::operator= (ControllerParam const& other)
{
	atomic (_parameter) = atomic (other._parameter);
	atomic (_smoothing_enabled) = atomic (other._smoothing_enabled);

	_minimum = other._minimum;
	_maximum = other._maximum;
	_denominator = other._denominator;
	_default_value = other._default_value;
	_smoothing_enabled = other._smoothing_enabled;

	return *this;
}


void
ControllerParam::save_state (QDomElement& element) const
{
	element.setAttribute ("value", QString ("%1").arg (get()));
	element.setAttribute ("smoothing-value", QString ("%1").arg (smoothing()));
	element.setAttribute ("smoothing-enabled", smoothing_enabled() ? "true" : "false");
}


void
ControllerParam::load_state (QDomElement const& element)
{
	set (bound (element.attribute ("value").toInt(), minimum(), maximum()));
	set_smoothing (bound (element.attribute ("smoothing-value").toInt(), 0, 10000));
	set_smoothing_enabled (element.attribute ("smoothing-enabled") == "true");
}

} // namespace Haruhi

