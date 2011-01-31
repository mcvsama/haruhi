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
#include <haruhi/utility/exception.h>

// Local:
#include "exception.h"
#include "buffer.h"
#include "graph.h"
#include "port.h"
#include "notification.h"
#include "unit.h"


namespace Haruhi {

int Unit::_id_counter = 0;


Unit::Unit (std::string const& urn, std::string const& title, int id):
	_graph (0),
	_synced (true),
	_enabled (false),
	_urn (urn),
	_title (title)
{
	set_id (id);
}


Unit::~Unit()
{
	// Prevent processing:
	_processing_mutex.lock();
	// Check if unit is properly disabled when destroyed:
	if (enabled())
		throw Exception ("disable unit before deletion");
	if (graph())
		throw Exception ("unregister unit before deletion");
	// Check if all ports have been unregistered:
	if (_inputs.size() > 0 || _outputs.size() > 0)
		throw Exception ("delete all ports before deleting unit");
	_processing_mutex.unlock();
}


void
Unit::sync()
{
	// Prevent syncing when not in processing round:
	if (!_graph->_inside_processing_round)
		throw OutsideProcessingRound ("tried to bump Sync outside processing round", __func__);

	// Check if we can acquire processing lock. If not, unit
	// is disabled - don't wait for it.
	// This prevents disabling unit when it's being processed.
	if (_processing_mutex.try_lock())
	{
		// Process:
		if (!_synced && _enabled)
		{
			_synced = true;
			this->process();
		}
		_processing_mutex.unlock();
	}
}


void
Unit::disable()
{
	// Wait for processing end:
	_processing_mutex.lock();
	_enabled = false;
	_processing_mutex.unlock();
}


void
Unit::set_id (int id)
{
	_id = id;
	if (_id <= 0)
		_id = Unit::allocate_id();
	else if (_id_counter < _id)
		_id_counter = _id;
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
Unit::graph_updated()
{
	graph()->lock();
	for (Ports::const_iterator i = _inputs.begin();  i != _inputs.end();  ++i)
		(*i)->graph_updated();
	for (Ports::const_iterator o = _outputs.begin();  o != _outputs.end();  ++o)
		(*o)->graph_updated();
	graph()->unlock();
}


int
Unit::allocate_id()
{
	return _id_counter += 1;
}


void
Unit::sync_inputs()
{
	for (Ports::const_iterator i = _inputs.begin();  i != _inputs.end();  ++i)
		(*i)->sync();
}

/**
 * Clears (prepares) buffers of all output ports.
 */
void
Unit::clear_outputs()
{
	for (Ports::const_iterator o = _outputs.begin();  o != _outputs.end();  ++o)
		(*o)->buffer()->clear();
}

} // namespace Haruhi

