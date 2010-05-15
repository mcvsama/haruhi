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
#include <string>

// Local:
#include "notification.h"


namespace Core {

Notification::Notification():
	_broadcast (true)
{ }


Notification::Notification (std::string const& target_urn):
	_broadcast (false),
	_target_urn (target_urn)
{ }

} // namespace Core

