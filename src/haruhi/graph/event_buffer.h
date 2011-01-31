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

#ifndef HARUHI__GRAPH__EVENT_BUFFER_H__INCLUDED
#define HARUHI__GRAPH__EVENT_BUFFER_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>
#include <list>

// Haruhi:
#include <haruhi/utility/memory.h>
#include <haruhi/utility/pool_allocator.h>

// Local:
#include "event.h"
#include "buffer.h"


namespace Haruhi {

class EventBuffer: public Buffer
{
	USES_POOL_ALLOCATOR (EventBuffer)

  public:
	typedef std::multiset<Shared<Event>, Event::SharedStrictWeakOrdering> EventsMultiset;

	static TypeID TYPE;

  public:
	EventBuffer();

	~EventBuffer();

	TypeID
	type() const { return EventBuffer::TYPE; }

	void
	clear() { _events->clear(); }

	/**
	 * Mixes in other buffer into this one.
	 * Other buffer must be static_castable to EventBuffer.
	 */
	void
	mixin (Buffer const*);

	void
	push (Shared<Event> const& event) { _events->insert (event); }

	EventsMultiset const&
	events() const { return *_events; }

	/**
	 * \returns	true if and only if buffer has no pending events.
	 */
	bool
	empty() const { return _events->empty(); }

  private:
	EventsMultiset* _events;
};

} // namespace Haruhi

#endif

