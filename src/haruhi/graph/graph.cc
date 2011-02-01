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

// System:
#include <sys/time.h>

// Haruhi:
#include <haruhi/utility/mutex.h>

// Local:
#include "graph.h"
#include "notification.h"
#include "audio_backend.h"
#include "event_backend.h"


namespace Haruhi {

Graph::Graph():
	RecursiveMutex(),
	_inside_processing_round (false),
	_buffer_size (0),
	_sample_rate (0),
	_tempo (120.0),
	_master_tune (440.0),
	_audio_backend (0),
	_event_backend (0)
{
	set_buffer_size (1);
}


Graph::~Graph()
{
}


void
Graph::register_audio_backend (AudioBackend* audio_backend)
{
	unregister_audio_backend();
	_audio_backend = audio_backend;
	register_unit (_audio_backend);
}


void
Graph::unregister_audio_backend()
{
	if (_audio_backend)
	{
		unregister_unit (_audio_backend);
		_audio_backend = 0;
	}
}


void
Graph::register_event_backend (EventBackend* event_backend)
{
	unregister_event_backend();
	_event_backend = event_backend;
	register_unit (_event_backend);
}


void
Graph::unregister_event_backend()
{
	if (_event_backend)
	{
		unregister_unit (_event_backend);
		_event_backend = 0;
	}
}


void
Graph::register_unit (Unit* unit)
{
	Graph* other_graph = unit->graph();
	if (other_graph)
	{
		other_graph->lock();
		other_graph->unregister_unit (unit);
		other_graph->unlock();
	}
	lock();
	unit->_graph = this;
	_units.insert (unit);
	unit->graph_updated();
	// Signal:
	unit_registered (unit);
	unlock();
	unit->registered();
}


void
Graph::unregister_unit (Unit* unit)
{
	Units::iterator f = _units.find (unit);
	if (f == _units.end())
		throw Exception ("unit was not registered in this graph");
	unit->disable();
	unit->unregistered();
	lock();
	// Notify ports about Unit unregistration:
	for (Ports::iterator p = unit->inputs().begin(); p != unit->inputs().end(); ++p)
		(*p)->unit_unregistered();
	for (Ports::iterator p = unit->outputs().begin(); p != unit->outputs().end(); ++p)
		(*p)->unit_unregistered();
	_units.erase (f);
	unit->_graph = 0;
	// Signal:
	unit_unregistered (unit);
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

} // namespace Haruhi
