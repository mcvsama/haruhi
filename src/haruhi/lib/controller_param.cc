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

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "controller_param.h"


namespace Haruhi {

ControllerParam::Adapter::Adapter (int limit_min, int limit_max):
	curve (0.0f),
	hard_limit_min (limit_min),
	hard_limit_max (limit_max),
	user_limit_min (limit_min),
	user_limit_max (limit_max)
{
}


int
ControllerParam::Adapter::encurve (int in) const
{
	float power = curve < 0
		? renormalize (curve, -1.0f, 0.0f, 0.4f, 1.0f)
		: renormalize (curve, 0.0f, +1.0f, 1.0f, 2.5f);
	return roundf (renormalize (std::pow (std::max (+0.0f, renormalize (in, 1.0f * hard_limit_min, 1.0f * hard_limit_max, 0.0f, 1.0f)), power),
								0.0f, 1.0f, 1.0f * hard_limit_min, 1.0f * hard_limit_max));
}


int
ControllerParam::Adapter::decurve (int in) const
{
	float power = curve < 0
		? renormalize (curve, -1.0f, 0.0f, 0.4f, 1.0f)
		: renormalize (curve, 0.0f, +1.0f, 1.0f, 2.5f);
	return roundf (renormalize (std::pow (renormalize (in, 1.0f * hard_limit_min, 1.0f * hard_limit_max, 0.0f, 1.0f), 1.0f / power),
								0.0f, 1.0f, 1.0f * hard_limit_min, 1.0f * hard_limit_max));
}


void
ControllerParam::save_state (QDomElement& element) const
{
	Param<int>::save_state (element);
	element.setAttribute ("curve", QString ("%1").arg (_adapter.curve, 0, 'f', 1));
	element.setAttribute ("user-limit-min", QString ("%1").arg (_adapter.user_limit_min));
	element.setAttribute ("user-limit-max", QString ("%1").arg (_adapter.user_limit_max));
}


void
ControllerParam::load_state (QDomElement const& element)
{
	_adapter.curve = 0.0;
	_adapter.user_limit_min = 0;
	_adapter.user_limit_max = 1;
	if (element.hasAttribute ("curve"))
		_adapter.curve = bound (element.attribute ("curve").toFloat(), -1.0f, 1.0f);
	if (element.hasAttribute ("user-limit-min"))
		_adapter.user_limit_min = bound (element.attribute ("user-limit-min").toInt(), _adapter.hard_limit_min, _adapter.hard_limit_max);
	if (element.hasAttribute ("user-limit-max"))
		_adapter.user_limit_max = bound (element.attribute ("user-limit-max").toInt(), _adapter.hard_limit_min, _adapter.hard_limit_max);
	Param<int>::load_state (element);
}

} // namespace Haruhi
