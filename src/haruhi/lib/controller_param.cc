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
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "controller_param.h"


namespace Haruhi {

void
ControllerParam::save_state (QDomElement& element) const
{
	element.setAttribute ("value", QString ("%1").arg (get()));
	element.appendChild (element.ownerDocument().createTextNode (QString::number (get())));
}


void
ControllerParam::load_state (QDomElement const& element)
{
	set (bound (element.text().toInt(), minimum(), maximum()));
}

} // namespace Haruhi

