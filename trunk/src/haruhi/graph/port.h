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

#ifndef HARUHI__GRAPH__PORT_H__INCLUDED
#define HARUHI__GRAPH__PORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <set>

// Haruhi:
#include <haruhi/utility/noncopyable.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/signal.h>

// Local:
#include "ports.h"
#include "event_backend.h"


namespace Haruhi {

class Unit;
class PortGroup;
class Buffer;

/**
 * Represents Port in Graph. Port is a data entry point for Units.
 * Unit can have any number of Ports associated with it.
 * Ports can be registered on Unit before Unit is registered to Graph.
 *
 * No method is thread-safe. You must lock graph
 * before using port's methods.
 */
class Port:
	public EventBackend::Learnable,
	private Noncopyable
{
	friend class Unit;
	friend class Graph;

  public:
	/**
	 * Is this input or output port from the owner unit view point?
	 */
	enum Direction { Input, Output };

	/**
	 * Flags describing port.
	 */
	enum {
		ControlKeyboard			= 1 << 0,
		ControlSustain			= 1 << 1,
		ControlPitchbend		= 1 << 2,
		ControlModulation		= 1 << 3,
		ControlVolume			= 1 << 4,
		ControlProgramChange	= 1 << 5,
		StandardAudio			= 1 << 6,
		Polyphonic				= 1 << 7,
	};

	typedef int Flags;

	/**
	 * Helper for sorting.
	 */
	struct CompareByName
	{
		bool
		operator() (Port* a, Port* b) const
		{
			return a->name() < b->name();
		}
	};

  public:
	Port (Unit* unit, std::string const& name, Direction, Buffer* buffer, PortGroup* group = 0, Flags flags = 0);

	/**
	 * Disconnects from any connected ports and deletes parameters if not null.
	 */
	virtual ~Port();

  public:
	/**
	 * \returns	owner of this port.
	 */
	Unit*
	unit() const { return _unit; }

	/**
	 * \returns	name of this port.
	 */
	std::string
	name() const { return _name; }

	/**
	 * \returns	comment for this port.
	 */
	std::string
	comment() const { return _comment; }

	/**
	 * \returns	fully qualified name (ie. group-name:port-name)
	 */
	std::string
	full_name() const;

	/**
	 * \returns	port direction.
	 */
	Direction
	direction() const { return _direction; }

	/**
	 * \returns	port's flags.
	 */
	Flags
	flags() const { return _flags; }

	/**
	 * \returns	true if port has all given flags.
	 */
	bool
	has_flags (Flags flags) { return (_flags & flags) == flags; }

	/**
	 * Sets new name for port.
	 * Triggers 'renamed' notification in graph.
	 */
	void
	set_name (std::string const& name);

	/**
	 * Sets new comment for the port.
	 * Triggers 'renamed' notification in graph.
	 */
	void
	set_comment (std::string const& comment);

	/**
	 * \returns	buffer for port, either port's own or by cascade to nearest buffer.
	 */
	Buffer*
	buffer() const { return _buffer; }

	/**
	 * \returns	group for this port.
	 */
	PortGroup*
	group() const { return _group; }

	/**
	 * \returns	set of back connections.
	 */
	Ports const&
	back_connections() const { return _back_connections; }

	/**
	 * \returns	set of forward connections.
	 */
	Ports const&
	forward_connections() const { return _forward_connections; }

	/**
	 * \returns	true if given ports are connected.
	 */
	bool
	connected_to (Port* other) const;

	/**
	 * Connects two ports.
	 */
	void
	connect_to (Port* other);

	/**
	 * Disconnects two ports.
	 */
	void
	disconnect_from (Port* other);

	/**
	 * Totally disconnects port from any other ports.
	 */
	void
	disconnect();

	/**
	 * Synces port.
	 */
	void
	sync();

	/**
	 * Puts port into 'learning' mode (as it is EventBackend::Learnable).
	 * It is required that port is owned by unit registered in Graph,
	 * and there is EventBackend registered in that Graph. If these conditions
	 * are not met, this method will throw GraphNotFound or EventBackendNotFound
	 * exceptions.
	 *
	 * \param	event_types Types of events that trigger 'learned' method.
	 */
	void
	start_learning (EventBackend::EventTypes event_types);

	/**
	 * Resets port from 'learning' mode (see start_learning()).
	 * It is required that port is owned by unit registered in Graph.
	 * If EventBackend is not registered in Graph, nothing will happen.
	 *
	 * \param	event_types Types of events that no longer will trigger 'learned' method.
	 * \throws	GraphNotFound if unit is not registered to any graph.
	 */
	void
	stop_learning();

	/**
	 * Shortcut for unit()->graph()
	 */
	Graph*
	graph() const;

	/**
	 * Called by unit when graph parameters change.
	 */
	virtual void
	graph_updated() = 0;

  protected:
	void
	register_me();

	void
	unregister_me();

  private:
	/**
	 * Callback of EventTeacher API.
	 * Connects learned port to this port, if Unit of this Port is still registered in Graph,
	 * and the Graph is the same as learned port's Graph.
	 */
	void
	learned_connection (EventBackend::EventTypes, EventPort* learned_port);

	/**
	 * Called by friendly Graph when Unit is unregistered. Causes total disconnection
	 * of this port and stops it from learning.
	 */
	void
	unit_unregistered();

  public:
	/**
	 * Called when port, after being in learning mode, is finally learned connection
	 * by EventBackend. Called always, even if actual connection hasn't been made
	 * because of distinct Graphs or other reasons.
	 *
	 * It is not defined from within what thread this signal will be emited.
	 */
	Signal::Emiter2<EventBackend::EventTypes, EventPort*> learned_connection_signal;

  private:
	Unit*			_unit;
	std::string		_name;
	std::string		_comment;
	Direction		_direction;
	Buffer*			_buffer;
	PortGroup*		_group;
	Flags			_flags;

	Ports			_back_connections;
	Ports			_forward_connections;
};

} // namespace Haruhi

#endif

