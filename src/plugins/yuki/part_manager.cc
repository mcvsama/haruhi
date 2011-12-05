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
#include <cstddef>
#include <algorithm>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "part_manager.h"
#include "part_manager_widget.h"


namespace Yuki {

PartManager::PartManager (Plugin* plugin):
	_plugin (plugin)
{
}


void
PartManager::add_part()
{
	Part* p = new Part (this);
	p->set_id (_id_alloc.allocate_id());
	_parts.push_back (p);
	widget()->add_part (p);
}


void
PartManager::remove_part (Part* part)
{
	_parts.remove (part);
	widget()->remove_part (part);
	_id_alloc.free_id (part->id());
	delete part;
}


void
PartManager::remove_all_parts()
{
	Parts ps = _parts;
	for (Parts::iterator p = ps.begin(); p != ps.end(); ++p)
		remove_part (*p);
}


void
PartManager::ensure_there_is_at_least_one_part()
{
	if (_parts.empty())
		add_part();
}


void
PartManager::set_part_position (Part* part, unsigned int position)
{
	_parts_mutex.lock();
	assert (position < _parts.size());
	Parts::iterator i = std::find (_parts.begin(), _parts.end(), part);
	Parts::iterator b = _parts.begin();
	std::advance (b, position);
	assert (i != _parts.end());
	assert (b != _parts.end());
	_parts.remove (*i);
	_parts.insert (b, *i);
	_parts_mutex.unlock();
}

} // namespace Yuki

