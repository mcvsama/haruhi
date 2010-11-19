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

// Local:
#include "event_backend.h"


namespace Haruhi {

EventBackend::EventBackend (std::string const& title, int id):
	Backend ("urn://haruhi.mulabs.org/backend/event-backend/1", title, id)
{
}


void
EventTeacher::start_learning (Learnable* learnable, EventTypes event_types)
{
	_learnables.insert (std::make_pair (learnable, event_types));
}


void
EventTeacher::stop_learning (Learnable* learnable, EventTypes event_types)
{
	_learnables.erase (std::make_pair (learnable, event_types));
}

} // namespace Haruhi

