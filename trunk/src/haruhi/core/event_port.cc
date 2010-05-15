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
#include "event_port.h"
#include "event_buffer.h"
#include "unit.h"


namespace Core {

EventPort::EventPort (Unit* unit, std::string const& name, Port::Direction direction, PortGroup* group, Flags flags):
	Port (unit, name, direction, new EventBuffer(), group, flags)
{
	register_me();
}


EventPort::~EventPort()
{
	unregister_me();
}


EventBuffer*
EventPort::event_buffer() const
{
	return static_cast<EventBuffer*> (buffer());
}


void
EventPort::graph_updated()
{
}

} // namespace Core

