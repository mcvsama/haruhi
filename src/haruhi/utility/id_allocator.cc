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
#include <cstddef>

// Local:
#include "id_allocator.h"


unsigned int
IDAllocator::allocate_id()
{
	unsigned int id = 1;
	for (; !_ids.insert (id).second; ++id)
		;
	return id;
}

unsigned int
IDAllocator::reserve_id (unsigned int id)
{
	return _ids.insert (id).second ? id : allocate_id();
}


void
IDAllocator::free_id (unsigned int id)
{
	_ids.erase (id);
}


void
IDAllocator::clear()
{
	_ids.clear();
}

