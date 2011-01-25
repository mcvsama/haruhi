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

// Local:
#include "event_backend.h"


namespace Haruhi {

EventTeacher::~EventTeacher()
{
	// Stop learning all learnables:
	Learnables lcopy = _learnables;
	for (Learnables::iterator l = lcopy.begin(); l != lcopy.end(); ++l)
		stop_learning (l->first);
}


void
EventTeacher::start_learning (Learnable* learnable, EventTypes event_types)
{
	_learnables.insert (std::make_pair (learnable, event_types));
}


void
EventTeacher::stop_learning (Learnable* learnable)
{
	_learnables.erase (learnable);
}


EventBackend::EventBackend (std::string const& title, int id):
	Backend ("urn://haruhi.mulabs.org/backend/event-backend/1", title, id)
{
}

} // namespace Haruhi

