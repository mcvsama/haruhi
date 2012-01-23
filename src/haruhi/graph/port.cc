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
#include <haruhi/utility/exception.h>

// Local:
#include "buffer.h"
#include "exception.h"
#include "graph.h"
#include "port.h"
#include "port_group.h"
#include "event_port.h"
#include "unit.h"


namespace Haruhi {

Port::Port (Unit* unit, std::string const& name, Direction direction, Buffer* buffer, PortGroup* group, Flags flags) noexcept:
	_unit (unit),
	_name (name),
	_direction (direction),
	_buffer (buffer),
	_group (group),
	_flags (flags)
{
}


std::string
Port::full_name() const
{
	return (_group ? _group->name() + ":" : "") + _name;
}


void
Port::set_name (std::string const& name)
{
	_name = name;
	if (graph())
		graph()->port_renamed (this);
}


void
Port::set_comment (std::string const& comment)
{
	_comment = comment;
	if (graph())
		graph()->port_renamed (this);
}


bool
Port::connected_to (Port* port) const noexcept
{
	Ports::const_iterator p;
	// False if 'this._forward_connections' doesn't contain 'other':
	p = _forward_connections.find (port);
	if (p == _forward_connections.end() || *p != port)
		return false;
	// False if 'port._back_connections' doesn't contain 'this':
	p = port->_back_connections.find (const_cast<Port*> (this));
	if (p == port->_back_connections.end() || *p != const_cast<Port*> (this))
		return false;
	// Otherwise true:
	return true;
}


void
Port::connect_to (Port* port)
{
	assert (this != port);
	// Skip if ports are already connected:
	if (!connected_to (port))
	{
		if (buffer()->type() != port->buffer()->type())
			throw PortIncompatible ("can't connect due to incompatible ports' buffers", __func__);
		// Add connections to maps:
		_forward_connections.insert (port);
		port->_back_connections.insert (this);
		if (graph())
			graph()->port_connected_to (this, port);
	}
}


void
Port::disconnect_from (Port* port)
{
	_forward_connections.erase (port);
	port->_back_connections.erase (this);
	if (graph())
		graph()->port_disconnected_from (this, port);
}


void
Port::disconnect()
{
	// Disconnect back connections; copy set, because its iterators will be invalidated when disconnecting.
	// Operate on copy.
	for (Port* p: Ports (_back_connections))
		p->disconnect_from (this);
	// Disconnect forward connections.
	// Operate on copy.
	for (Port* p: Ports (_forward_connections))
		this->disconnect_from (p);
}


void
Port::sync()
{
	if (unit()->enabled())
	{
		if (_direction == Output)
			unit()->sync();
		else if (_direction == Input)
		{
			buffer()->clear();
			if (_back_connections.empty())
				no_input();
			else
			{
				for (Port* p: _back_connections)
				{
					p->sync();
					buffer()->mixin (p->buffer());
				}
			}
		}
	}
	else
		buffer()->clear();
}


void
Port::start_learning (EventBackend::EventTypes)
{
	Graph* graph = this->graph();
	assert (graph != nullptr, "port must be registered to graph before it can learn/stop learning connections");
	EventBackend* event_backend = graph->event_backend();
	assert (event_backend != nullptr, "graph must have event backend registered before port can learn/stop learning connections");
	event_backend->start_learning (this, EventBackend::Controller | EventBackend::Pitchbend);
}


void
Port::stop_learning()
{
	Graph* graph = this->graph();
	assert (graph != nullptr, "port must be registered to graph before it can learn/stop learning connections");
	EventBackend* event_backend = graph->event_backend();
	if (!event_backend)
		return;
	event_backend->stop_learning (this);
}


void
Port::register_me()
{
	switch (_direction)
	{
		case Port::Input:	_unit->_inputs.insert (this);	break;
		case Port::Output:	_unit->_outputs.insert (this);	break;
	}
	// Send notification:
	if (graph())
		graph()->port_registered (this, _unit);
}


void
Port::unregister_me()
{
	disconnect();
	switch (_direction)
	{
		case Port::Input:	_unit->_inputs.erase (this);	break;
		case Port::Output:	_unit->_outputs.erase (this);	break;
	}
	// Send notification:
	if (graph())
		graph()->port_unregistered (this, _unit);
}


void
Port::learned_connection (EventBackend::EventTypes event_types, EventPort* learned_port)
{
	if (graph() && graph() == learned_port->graph())
	{
		graph()->synchronize ([&]() {
			learned_port->connect_to (this);
		});
	}
	// Emit signal:
	learned_connection_signal (event_types, learned_port);
}


void
Port::unit_unregistered()
{
	disconnect();
	stop_learning();
}

} // namespace Haruhi

