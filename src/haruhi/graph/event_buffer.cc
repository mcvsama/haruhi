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

// Standard:
#include <iterator>
#include <algorithm>
#include <ext/algorithm>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/pool_allocator.h>

// Local:
#include "event_buffer.h"


namespace Haruhi {

POOL_ALLOCATOR_FOR (EventBuffer)

Buffer::TypeID EventBuffer::TYPE = "Haruhi::EventBuffer";


EventBuffer::EventBuffer() noexcept:
	Buffer (EventBuffer::TYPE),
	_sorted (true)
{
}


void
EventBuffer::mixin (Buffer const* other)
{
	assert (other->type() == EventBuffer::TYPE);
	auto other_buffer = static_cast<EventBuffer const*> (other);
	_events.insert (_events.end(), other_buffer->_events.begin(), other_buffer->_events.end());
}

} // namespace Haruhi

