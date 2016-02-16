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
#include <algorithm>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "buffer.h"
#include "graph.h"
#include "port.h"
#include "notification.h"
#include "unit.h"


namespace Haruhi {

Unit::IDs Unit::_ids;


Unit::Unit (std::string const& urn, std::string const& title, int id) noexcept:
	_graph (0),
	_synced (true),
	_enabled (false),
	_urn (urn),
	_title (title),
	_original_title (title)
{
	set_id (id);
}


Unit::~Unit()
{
	free_id (id());
	// Prevent processing:
	Mutex::Lock lock (_processing_mutex);
	// Check if unit is properly disabled when destroyed:
	assert (!enabled(), "disable unit before deletion");
	assert (!graph(), "unregister unit before deletion");
	// Check if all ports have been unregistered:
	if (!_inputs.empty() || !_outputs.empty())
	{
		std::string ports;
		if (!_inputs.empty())
		{
			ports += "inputs: ";
			for (Ports::iterator p = _inputs.begin(); p != _inputs.end(); ++p)
				ports += (p == _inputs.begin() ? "" : ", ") + (*p)->name();
		}
		if (!_outputs.empty())
		{
			if (!ports.empty())
				ports += "; ";
			ports += "outputs: ";
			for (Ports::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
				ports += (p == _outputs.begin() ? "" : ", ") + (*p)->name();
		}
		assert (_inputs.empty() && _outputs.empty(), ("delete all ports before deleting unit (" + ports + ")").c_str());
	}
}


void
Unit::sync()
{
	// Prevent syncing when not in processing round:
	assert (_graph->_inside_processing_round, "tried to bump Sync outside processing round");

	// Check if we can acquire processing lock. If not, unit
	// is disabled - don't wait for it.
	// This prevents disabling unit when it's being processed.
	Mutex::TryLock lock (_processing_mutex);
	if (lock.acquired() && !_synced && _enabled)
	{
		_synced = true;
		this->process();
	}
}


void
Unit::disable() noexcept
{
	// Wait for processing end:
	Mutex::Lock lock (_processing_mutex);
	_enabled = false;
}


void
Unit::set_id (int id)
{
	_id = id;
	if (_id <= 0)
		_id = Unit::allocate_id();
}


void
Unit::set_title (std::string const& title)
{
	_title = title;
	if (graph())
		graph()->unit_retitled (this);
}


void
Unit::panic()
{
}


void
Unit::sync_inputs()
{
	for (Port* p: _inputs)
		p->sync();
}

/**
 * Clears (prepares) buffers of all output ports.
 */
void
Unit::clear_outputs()
{
	for (Port* p: _outputs)
		p->clear_buffer();
}


void
Unit::graph_updated()
{
	Mutex::Lock lock (*graph());
	for (Port* p: _inputs)
		p->graph_updated();
	for (Port*p: _outputs)
		p->graph_updated();
}


int
Unit::allocate_id()
{
	// Find lowest free ID:
	int id = 1;
	while (!_ids.insert (id).second)
		++id;
	return id;
}


int
Unit::reserve_id (int id)
{
	return _ids.insert (id).second ? id : allocate_id();
}


void
Unit::free_id (int id)
{
	_ids.erase (id);
}

} // namespace Haruhi

