/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__GRAPH__EVENT_PORT_H__INCLUDED
#define HARUHI__GRAPH__EVENT_PORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/noncopyable.h>

// Local:
#include "event_buffer.h"
#include "graph.h"
#include "port.h"
#include "port_group.h"


namespace Haruhi {

class EventPort: public Port
{
  public:
	/**
	 * Locks Graph for operation - it is safe to create new EventPort
	 * without need to explicitly locking Graph before.
	 * \entry	Any thread.
	 */
	EventPort (Unit* unit, std::string const& name, Port::Direction direction, PortGroup* group = 0, Flags flags = 0, Tags tags = {});

	/**
	 * Locks Graph for operation - it is safe to delete AudioPort
	 * without need to explicitly locking Graph before.
	 * \entry	Any thread
	 */
	~EventPort();

	/**
	 * Helper that casts Buffer to EventBuffer.
	 */
	EventBuffer*
	buffer() const noexcept;

	/**
	 * Return default value set for port.
	 */
	ControllerEvent::Value
	default_value() const noexcept;

	/**
	 * Set default ControllerEvent that will be inserted into buffer,
	 * when nothing is connected to the Input port.
	 */
	void
	set_default_value (ControllerEvent::Value value) noexcept;

	/**
	 * Disable default value set with set_default_value().
	 */
	void
	disable_default_value() noexcept;

	/*
	 * Port implementation
	 */

	void
	clear_buffer() override;

	void
	mixin (Port*) override;

	void
	graph_updated() override { }

  protected:
	void
	no_input() override;

  private:
	bool					_default_value_set;
	ControllerEvent::Value	_default_value;
	Unique<EventBuffer>		_buffer;
};


inline EventBuffer*
EventPort::buffer() const noexcept
{
	return _buffer.get();
}


inline ControllerEvent::Value
EventPort::default_value() const noexcept
{
	return _default_value;
}


inline void
EventPort::set_default_value (ControllerEvent::Value value) noexcept
{
	_default_value = value;
	_default_value_set = true;
}


inline void
EventPort::disable_default_value() noexcept
{
	_default_value_set = false;
}

} // namespace Haruhi

#endif

