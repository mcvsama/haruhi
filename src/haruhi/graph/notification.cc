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
#include <string>

// Local:
#include "notification.h"


namespace Haruhi {

class Unit;

Notification::Notification (Unit* sender):
	_sender (sender),
	_broadcast (true)
{ }


Notification::Notification (Unit* sender, std::string const& target_urn):
	_sender (sender),
	_broadcast (false),
	_target_urn (target_urn)
{ }

} // namespace Haruhi
