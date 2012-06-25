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

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "controller_param.h"


namespace Haruhi {

ControllerParam::Adapter::Adapter (Range<int> limit, int center_value) noexcept:
	curve (0.0f),
	hard_limit (limit),
	user_limit (limit),
	_center (center_value)
{
}


int
ControllerParam::Adapter::encurve (int in) const noexcept
{
	Range<float> hard_range (hard_limit.min(), _center);
	Range<float> temp_range (1.0f, 0.0f);
	if (in >= _center)
	{
		hard_range = Range<float> (_center, hard_limit.max());
		temp_range.flip();
	}
	float const power = (curve < 0)
		? renormalize (curve, -1.0f, +0.0f, 0.4f, 1.0f)
		: renormalize (curve, +0.0f, +1.0f, 1.0f, 2.5f);
	float const a = renormalize (in, hard_range, temp_range);
	float const b = std::pow (std::max (0.0f, a), power);
	float const c = renormalize (b, temp_range, hard_range);
	return roundf (c);
}


int
ControllerParam::Adapter::decurve (int in) const noexcept
{
	Range<float> hard_range (hard_limit.min(), _center);
	Range<float> temp_range (1.0f, 0.0f);
	if (in >= _center)
	{
		hard_range = Range<float> (_center, hard_limit.max());
		temp_range.flip();
	}
	float const power = (curve < 0)
		? renormalize (curve, -1.0f, +0.0f, 0.4f, 1.0f)
		: renormalize (curve, +0.0f, +1.0f, 1.0f, 2.5f);
	float const a = renormalize (in, hard_range, temp_range);
	float const b = std::pow (a, 1.0f / power);
	float const c = renormalize (b, temp_range, hard_range);
	return roundf (c);
}


void
ControllerParam::save_state (QDomElement& element) const
{
	Param<int>::save_state (element);
	element.setAttribute ("curve", QString ("%1").arg (_adapter.curve, 0, 'f', 1));
	element.setAttribute ("user-limit-min", QString ("%1").arg (_adapter.user_limit.min()));
	element.setAttribute ("user-limit-max", QString ("%1").arg (_adapter.user_limit.max()));
}


void
ControllerParam::load_state (QDomElement const& element)
{
	_adapter.curve = 0.0;
	_adapter.user_limit.set_min (0);
	_adapter.user_limit.set_max (1);
	if (element.hasAttribute ("curve"))
		_adapter.curve = bound (element.attribute ("curve").toFloat(), -1.0f, 1.0f);
	if (element.hasAttribute ("user-limit-min"))
		_adapter.user_limit.set_min (bound (element.attribute ("user-limit-min").toInt(), _adapter.hard_limit.min(), _adapter.hard_limit.max()));
	if (element.hasAttribute ("user-limit-max"))
		_adapter.user_limit.set_max (bound (element.attribute ("user-limit-max").toInt(), _adapter.hard_limit.min(), _adapter.hard_limit.max()));
	Param<int>::load_state (element);
	sanitize();
}


void
ControllerParam::sanitize()
{
	Param<int>::sanitize();
	limit_value (_adapter.curve, -1.0f, +1.0f);
	_adapter.hard_limit.set_min (bound (_adapter.hard_limit.min(), range()));
	_adapter.hard_limit.set_max (bound (_adapter.hard_limit.max(), _adapter.hard_limit.min(), maximum()));
	_adapter.user_limit.set_min (bound (_adapter.user_limit.min(), range()));
	_adapter.user_limit.set_max (bound (_adapter.user_limit.max(), _adapter.user_limit.min(), maximum()));
}

} // namespace Haruhi
