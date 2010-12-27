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
#include <haruhi/config/all.h>
#include <haruhi/utility/backtrace.h>

// Local:
#include "settings.h"


namespace Haruhi {

namespace DevicesManager {

Settings::Settings():
	Module ("devices-manager")
{
}


void
Settings::save_state (QDomElement& element) const
{
	_model.save_state (element);
}


void
Settings::load_state (QDomElement const& element)
{
	_model.load_state (element);
}

} // namespace DevicesManager

} // namespace Haruhi

