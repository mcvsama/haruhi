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

#ifndef HARUHI__GRAPH__PORT_H__INCLUDED
#define HARUHI__GRAPH__PORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <set>

// Haruhi:
#include <haruhi/config/all.h>
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
		/**
		 * As audio ports can be used not only for audio data, it's good idea
		 * to mark normal audio ports with this flag.
		 */
		StandardAudio			= 1 << 0,

		/**
		 * This port outputs polyphonic events or can handle input polyphonic events.
		 * Polyphonic event is an event that has voice associated with it. It changes
		 * of a one, specific voice currently sounding.
		 */
		Polyphonic				= 1 << 1,

		/**
		 * TODO implement HD ports.
		 *
		 * This port is a HD port, that is it can generate HD data or make use
		 * of input HD data (see description of ControllerEvent).
		 *
		 * HD data is simply buffer of controller values that can be send with each ControllerEvent.
		 * Buffer size (number of values) must be equal to current Graph.buffer_size().
		 *
		 * HD ports are used by high-frequency LFOs and precise envelope generators,
		 * that can create series of controller values, each corresponding to one audio
		 * sample generated by the receiver (synthesizer, effect, etc.)
		 *
		 * Output ports can use Port.hd_receivers() to get number of connected input HD ports.
		 * If this number is = 0, no HD data is necessary, otherwise output port should generate
		 * HD data buffers for each appropriate output event.
		 */
		HighDefinition			= 1 << 2,
	};

	typedef int						Flags;
	typedef std::set<std::string>	Tags;

  public:
	Port (Unit* unit, std::string const& name, Direction, Buffer* buffer, PortGroup* group = 0, Flags flags = 0, Tags tags = {}) noexcept;

	/**
	 * Disconnects from any connected ports and deletes parameters if not null.
	 */
	// FIXME change to "= default" in new GCC.
	virtual ~Port() { }

  public:
	/**
	 * \returns	owner of this port.
	 */
	Unit*
	unit() const noexcept;

	/**
	 * \returns	name of this port.
	 */
	std::string
	name() const noexcept;

	/**
	 * \returns	comment for this port.
	 */
	std::string
	comment() const noexcept;

	/**
	 * \returns	fully qualified name (ie. group-name:port-name)
	 */
	std::string
	full_name() const;

	/**
	 * \returns	port direction.
	 */
	Direction
	direction() const noexcept;

	/**
	 * \returns	port's flags.
	 */
	Flags
	flags() const noexcept;

	/**
	 * \returns	true if port has all given flags.
	 */
	bool
	has_flags (Flags flags) const noexcept;

	/**
	 * Return tags set.
	 */
	Tags
	tags() const noexcept;

	/**
	 * Return true if port has all given tags.
	 */
	bool
	has_tag (Tags::value_type tag) const noexcept;

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
	buffer() const noexcept;

	/**
	 * \returns	group for this port.
	 */
	PortGroup*
	group() const noexcept;

	/**
	 * \returns	set of back connections.
	 */
	Ports const&
	back_connections() const noexcept;

	/**
	 * \returns	set of forward connections.
	 */
	Ports const&
	forward_connections() const noexcept;

	/**
	 * \returns	true if given ports are connected.
	 */
	bool
	connected_to (Port* other) const noexcept;

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
	 * and there is EventBackend registered in that Graph.
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
	 */
	void
	stop_learning();

	/**
	 * Shortcut for unit()->graph()
	 */
	Graph*
	graph() const noexcept;

	/**
	 * Called by unit when graph parameters change.
	 */
	virtual void
	graph_updated() = 0;

	/**
	 * Helper for sorting.
	 */
	static bool
	compare_by_name (Port const* first, Port const* second) noexcept;

  protected:
	void
	register_me();

	void
	unregister_me();

	/**
	 * Called when sync() is requested, but nothing is connected
	 * to the port. Default implementation does nothing.
	 */
	virtual void
	no_input() { }

  private:
	/**
	 * Callback of EventTeacher API.
	 * Connects learned port to this port, if Unit of this Port is still registered in Graph,
	 * and the Graph is the same as learned port's Graph.
	 */
	void
	learned_connection (EventBackend::EventTypes, EventPort* learned_port) override;

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
	Tags			_tags;

	Ports			_back_connections;
	Ports			_forward_connections;
};


inline Unit*
Port::unit() const noexcept
{
	return _unit;
}


inline std::string
Port::name() const noexcept
{
	return _name;
}


inline std::string
Port::comment() const noexcept
{
	return _comment;
}


inline Port::Direction
Port::direction() const noexcept
{
	return _direction;
}


inline Port::Flags
Port::flags() const noexcept
{
	return _flags;
}


inline bool
Port::has_flags (Flags flags) const noexcept
{
	return (_flags & flags) == flags;
}


inline Port::Tags
Port::tags() const noexcept
{
	return _tags;
}


inline bool
Port::has_tag (Tags::value_type tag) const noexcept
{
	return _tags.find (tag) != _tags.end();
}


inline Buffer*
Port::buffer() const noexcept
{
	return _buffer;
}


inline PortGroup*
Port::group() const noexcept
{
	return _group;
}


inline Ports const&
Port::back_connections() const noexcept
{
	return _back_connections;
}


inline Ports const&
Port::forward_connections() const noexcept
{
	return _forward_connections;
}


inline Graph*
Port::graph() const noexcept
{
	return _unit->graph();
}


inline bool
Port::compare_by_name (Port const* first, Port const* second) noexcept
{
	return first->name() < second->name();
}

} // namespace Haruhi

#endif

