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

// Standard:
#include <iterator>
#include <algorithm>
#include <ext/algorithm>

// Haruhi:
#include <haruhi/utility/pool_allocator.h>

// Local:
#include "event_buffer.h"
#include "exception.h"


namespace Haruhi {

POOL_ALLOCATOR_FOR (EventBuffer)

Buffer::TypeID EventBuffer::TYPE = "Haruhi::EventBuffer";


EventBuffer::EventBuffer():
	_events (new EventsMultiset())
{
}


EventBuffer::~EventBuffer()
{
	delete _events;
}


void
EventBuffer::mixin (Buffer* other)
{
	if (other->type() != EventBuffer::TYPE)
		throw Exception ("incompatible buffers");
	EventBuffer const* other_buffer = static_cast<EventBuffer*> (other);
	EventsMultiset* aux = new EventsMultiset();
	std::merge (_events->begin(), _events->end(), other_buffer->_events->begin(), other_buffer->_events->end(),
				std::insert_iterator<EventsMultiset> (*aux, aux->begin()), EventsMultiset::key_compare());
	delete _events;
	_events = aux;
}

} // namespace Haruhi

