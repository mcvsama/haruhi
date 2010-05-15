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

#ifndef HARUHI__CORE__EVENT_PORT_H__INCLUDED
#define HARUHI__CORE__EVENT_PORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/utility/noncopyable.h>

// Local:
#include "graph.h"
#include "port.h"
#include "port_group.h"


namespace Core {

class EventBuffer;


class EventPort: public Port
{
  public:
	EventPort (Unit* unit, std::string const& name, Port::Direction direction, PortGroup* group = 0, Flags flags = 0);

	~EventPort();

	/**
	 * Helper that casts Buffer to EventBuffer.
	 */
	EventBuffer*
	event_buffer() const;

	/**
	 * Implementation of Port::graph_updated().
	 */
	void
	graph_updated();
};

} // namespace Core

#endif

