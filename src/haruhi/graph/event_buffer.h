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
	typedef std::vector<Shared<Event> > Events;

	static TypeID TYPE;

  public:
	EventBuffer();

	void
	clear();

	/**
	 * Mixes in other buffer into this one.
	 * Other buffer must be static_castable to EventBuffer.
	 */
	void
	mixin (Buffer const*);

	void
	push (Shared<Event> const& event);

	/**
	 * May resort events.
	 */
	Events&
	events();

	Events const&
	events() const;

	/**
	 * \returns	true if and only if buffer has no pending events.
	 */
	bool
	empty() const;

  private:
	void
	ensure_sorted() const;

  private:
	mutable bool	_sorted;
	mutable Events	_events;
};


inline void
EventBuffer::clear()
{
	_events.clear();
}


inline void
EventBuffer::push (Shared<Event> const& event)
{
	_sorted = false; _events.push_back (event);
}


inline EventBuffer::Events&
EventBuffer::events()
{
	ensure_sorted();
	return _events;
}


inline EventBuffer::Events const&
EventBuffer::events() const
{
	ensure_sorted();
	return _events;
}


inline bool
EventBuffer::empty() const
{
	return _events.empty();
}


inline void
EventBuffer::ensure_sorted() const
{
	if (!_sorted)
	{
		std::sort (_events.begin(), _events.end(), Event::shared_strict_weak_ordering);
		_sorted = true;
	}
}

} // namespace Haruhi

#endif

