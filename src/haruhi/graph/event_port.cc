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

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "event_port.h"
#include "unit.h"


namespace Haruhi {

EventPort::EventPort (Unit* unit, std::string const& name, Port::Direction direction, PortGroup* group, Flags flags):
	Port (unit, name, direction, new EventBuffer(), group, flags),
	_default_value_set (false),
	_default_value()
{
	Graph* g = graph();
	if (g)
		g->lock();
	register_me();
	if (g)
		g->unlock();
}


EventPort::~EventPort()
{
	Graph* g = graph();
	if (g)
		g->lock();
	unregister_me();
	if (g)
		g->unlock();
}


void
EventPort::no_input()
{
	if (_default_value_set)
		event_buffer()->push (new ControllerEvent (Graph::now(), _default_value));
}

} // namespace Haruhi

