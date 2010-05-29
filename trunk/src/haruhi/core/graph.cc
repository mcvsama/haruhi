/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
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

// System:
#include <sys/time.h>

// Haruhi:
#include <haruhi/utility/mutex.h>

// Local:
#include "graph.h"
#include "notification.h"


namespace Core {

Graph::Graph():
	RecursiveMutex(),
	_inside_processing_round (false),
	_buffer_size (0),
	_sample_rate (0),
	_tempo (120.0),
	_master_tune (440.0)
{
	set_buffer_size (1);
}


Graph::~Graph()
{
}


void
Graph::register_unit (Unit* unit)
{
	Graph* other = unit->graph();
	if (other)
	{
		other->lock();
		other->unregister_unit (unit);
		other->unlock();
	}
	lock();
	unit->_graph = this;
	_units.insert (unit);
	// Signal:
	unit_registered (unit);
	unlock();
}


void
Graph::unregister_unit (Unit* unit)
{
	lock();
	Units::iterator f = _units.find (unit);
	if (f != _units.end())
	{
		_units.erase (f);
		unit->_graph = 0;
		// Signal:
		unit_unregistered (unit);
	}
	unlock();
}


void
Graph::enter_processing_round()
{
	lock();
	_timestamp = Graph::now();
	_inside_processing_round = true;
	_dummy_syncing = false;
	// Wakeup all Units:
	for (Units::iterator k = _units.begin();  k != _units.end();  ++k)
		(*k)->_synced = false;
}


void
Graph::leave_processing_round()
{
	_dummy_syncing = true;
	// Bump only unsynced Units:
	for (Units::iterator k = _units.begin();  k != _units.end();  ++k)
		if (!(*k)->_synced && (*k)->_enabled)
			(*k)->sync();
	_inside_processing_round = false;
	unlock();
}


void
Graph::panic()
{
	for (Units::iterator k = _units.begin();  k != _units.end();  ++k)
		(*k)->panic();
}


void
Graph::set_buffer_size (std::size_t buffer_size)
{
	_buffer_size = buffer_size;
	for (Units::iterator k = _units.begin();  k != _units.end();  ++k)
		(*k)->graph_updated();
}


void
Graph::set_sample_rate (std::size_t sample_rate)
{
	_sample_rate = sample_rate;
	for (Units::iterator k = _units.begin();  k != _units.end();  ++k)
		(*k)->graph_updated();
}


void
Graph::set_tempo (float tempo)
{
	_tempo = tempo;
	for (Units::iterator k = _units.begin();  k != _units.end();  ++k)
		(*k)->graph_updated();
}


void
Graph::set_master_tune (float master_tune)
{
	_master_tune = master_tune;
	for (Units::iterator k = _units.begin();  k != _units.end();  ++k)
		(*k)->graph_updated();
}


void
Graph::notify (Notification* notification)
{
	for (Units::iterator k = _units.begin();  k != _units.end();  ++k)
		(*k)->notify (notification);
	delete notification;
}


Graph::Units const&
Graph::units() const
{
	return _units;
}


Timestamp
Graph::now()
{
	struct timeval t;
	::gettimeofday (&t, 0);
	return t.tv_sec * 1000000 + t.tv_usec;
}

} // namespace Core

