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
#include <haruhi/utility/numeric.h>

// Local:
#include "controller_param.h"


namespace Haruhi {

ControllerParam::ControllerParam():
	_denominator (1),
	_smoothing_enabled (false),
	_smoothing_parameter (0)
{ }


ControllerParam::ControllerParam (int minimum, int maximum, int default_value, int denominator):
	Param (minimum, maximum, default_value),
	_denominator (denominator),
	_smoothing_enabled (false),
	_smoothing_parameter (0)
{ }


ControllerParam&
ControllerParam::operator= (ControllerParam const& other)
{
	Param::operator= (other);

	_denominator = other._denominator;
	_smoothing_enabled = other._smoothing_enabled;
	atomic (_smoothing_parameter) = static_cast<int> (atomic (other._smoothing_parameter));

	return *this;
}


void
ControllerParam::sanitize()
{
	Param::sanitize();
	set_smoothing (bound (smoothing(), 0, 1000)); // 0…1000ms
}


void
ControllerParam::save_state (QDomElement& element) const
{
	element.setAttribute ("value", QString ("%1").arg (get()));
	element.setAttribute ("smoothing-value", QString ("%1").arg (smoothing()));
	element.setAttribute ("smoothing-enabled", smoothing_enabled() ? "true" : "false");
	element.appendChild (element.ownerDocument().createTextNode (QString::number (get())));
}


void
ControllerParam::load_state (QDomElement const& element)
{
	set_smoothing (0);
	set_smoothing_enabled (false);
	if (element.hasAttribute ("smoothing-value"))
		set_smoothing (bound (element.attribute ("smoothing-value").toInt(), 0, 10000));
	if (element.hasAttribute ("smoothing-enabled"))
		set_smoothing_enabled (element.attribute ("smoothing-enabled") == "true");
	set (bound (element.text().toInt(), minimum(), maximum()));
}

} // namespace Haruhi

