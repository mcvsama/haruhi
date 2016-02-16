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

#ifndef HARUHI__GRAPH__EVENT_BUFFER_H__INCLUDED
#define HARUHI__GRAPH__EVENT_BUFFER_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>
#include <list>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/memory.h>
#include <haruhi/utility/pool_allocator.h>

// Local:
#include "event.h"


namespace Haruhi {

class EventBuffer
{
	USES_POOL_ALLOCATOR (EventBuffer)

  public:
	typedef std::vector<Shared<Event>> Events;

  public:
	EventBuffer() noexcept;

	void
	clear() noexcept;

	/**
	 * Mixes in other buffer into this one.
	 * Other buffer must be static_castable to EventBuffer.
	 */
	void
	mixin (EventBuffer const*);

	void
	push (Shared<Event> const& event);

	/**
	 * May resort events.
	 */
	Events&
	events() noexcept;

	Events const&
	events() const noexcept;

	/**
	 * \returns	true if and only if buffer has no pending events.
	 */
	bool
	empty() const noexcept;

  private:
	void
	ensure_sorted() const noexcept;

  private:
	mutable bool	_sorted;
	mutable Events	_events;
};


inline void
EventBuffer::clear() noexcept
{
	_events.clear();
}


inline void
EventBuffer::push (Shared<Event> const& event)
{
	_sorted = false;
	_events.push_back (event);
}


inline EventBuffer::Events&
EventBuffer::events() noexcept
{
	ensure_sorted();
	return _events;
}


inline EventBuffer::Events const&
EventBuffer::events() const noexcept
{
	ensure_sorted();
	return _events;
}


inline bool
EventBuffer::empty() const noexcept
{
	return _events.empty();
}


inline void
EventBuffer::ensure_sorted() const noexcept
{
	if (!_sorted)
	{
		std::sort (_events.begin(), _events.end(), Event::shared_strict_weak_ordering);
		_sorted = true;
	}
}

} // namespace Haruhi

#endif

