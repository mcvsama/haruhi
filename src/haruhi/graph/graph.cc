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

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/mutex.h>

// Local:
#include "graph.h"
#include "notification.h"
#include "audio_backend.h"
#include "event_backend.h"


namespace Haruhi {

Graph::Graph()
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
		Mutex::Lock lock (*other_graph);
		other_graph->unregister_unit (unit);
	}
	synchronize ([&] {
		unit->_graph = this;
		_units.insert (unit);
		unit->graph_updated();
		// Signal:
		unit_registered (unit);
	});
	unit->registered();
}


void
Graph::unregister_unit (Unit* unit)
{
	Units::iterator f = _units.find (unit);
	assert (f != _units.end());
	unit->disable();
	unit->unregistered();

	Mutex::Lock lock (*this);
	// Notify ports about Unit unregistration:
	for (Port* p: unit->inputs())
		p->unit_unregistered();
	for (Port* p: unit->outputs())
		p->unit_unregistered();
	_units.erase (f);
	unit->_graph = 0;
	// Signal:
	unit_unregistered (unit);
}


void
Graph::enter_processing_round()
{
	lock();
	_timestamp = Time::now();
	_inside_processing_round = true;
	_dummy_syncing = false;
	// Wakeup all Units:
	for (Unit* u: _units)
		u->_synced = false;
}


void
Graph::leave_processing_round()
{
	_dummy_syncing = true;
	// Bump only unsynced Units:
	for (Unit* u: _units)
		if (!u->_synced && u->_enabled)
			u->sync();
	_inside_processing_round = false;
	compute_next_tempo_tick();
	unlock();
}


void
Graph::panic()
{
	for (Unit* u: _units)
		u->panic();
}


void
Graph::set_buffer_size (std::size_t buffer_size)
{
	_buffer_size = buffer_size;
	for (Unit* u: _units)
		u->graph_updated();
}


void
Graph::set_sample_rate (Frequency sample_rate)
{
	_sample_rate = sample_rate;
	for (Unit* u: _units)
		u->graph_updated();
}


void
Graph::set_tempo (Frequency tempo)
{
	_tempo = tempo;
	for (Unit* u: _units)
		u->graph_updated();
}


void
Graph::set_master_tune (Frequency master_tune)
{
	_master_tune = master_tune;
	for (Unit* u: _units)
		u->graph_updated();
}


void
Graph::notify (Unique<Notification> notification)
{
	for (Unit* u: _units)
		u->notify (notification.get());
}


void
Graph::compute_next_tempo_tick()
{
	if (_next_tempo_tick >= _buffer_size)
		_next_tempo_tick -= _buffer_size;
	else
		_next_tempo_tick = _sample_rate / _tempo;
}

} // namespace Haruhi

