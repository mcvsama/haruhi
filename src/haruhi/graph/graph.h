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

#ifndef HARUHI__GRAPH__GRAPH_H__INCLUDED
#define HARUHI__GRAPH__GRAPH_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/signal.h>

// Local:
#include "unit.h"


namespace Haruhi {

class Port;
class Unit;
class PortGroup;
class Notification;
class AudioBackend;
class EventBackend;

class Graph: public RecursiveMutex
{
	friend class Unit;

  public:
	typedef std::set<Unit*> Units;

  public:
	Graph();

	virtual ~Graph();

	/**
	 * Registers unit as audio backend in graph.
	 * Graph does not take ownership of the unit.
	 * Registered backend is also registered as regular Unit in Graph.
	 */
	void
	register_audio_backend (AudioBackend*);

	/**
	 * Deregisters audio backend.
	 * Does nothing if there's no audio backend registered.
	 */
	void
	unregister_audio_backend();

	/**
	 * Registers unit as event backend in graph.
	 * Graph does not take ownership of the unit.
	 * Registered backend is also registered as regular Unit in Graph.
	 */
	void
	register_event_backend (EventBackend*);

	/**
	 * Deregisters event backend.
	 * Does nothing if there's no event backend registered.
	 */
	void
	unregister_event_backend();

	/**
	 * Returns currently registered audio backend (or 0 if there's none).
	 */
	AudioBackend*
	audio_backend() const noexcept;

	/**
	 * Returns currently registered event backend (or 0 if there's none).
	 */
	EventBackend*
	event_backend() const noexcept;

	/**
	 * Registers unit in this graph.
	 */
	void
	register_unit (Unit*);

	/**
	 * Unregisters unit from this graph.
	 */
	void
	unregister_unit (Unit*);

	/**
	 * Changes state of all Synces to 'ready-for-bump'.
	 * Should be called at the beginning of real processing round
	 * (created by audio backend for example).
	 */
	void
	enter_processing_round();

	/**
	 * Causes all objects that were not 'bumped' to process data.
	 * Should be called at the end of real processing round
	 * (created by audio backend for example).
	 */
	void
	leave_processing_round();

	/**
	 * Calls panic() on all registered units.
	 */
	void
	panic();

	/**
	 * Returns buffer size for audio buffers.
	 */
	std::size_t
	buffer_size() const noexcept;

	/**
	 * Sets buffer size and updates all connected port buffers.
	 */
	void
	set_buffer_size (std::size_t buffer_size);

	/**
	 * Returns current sample rate.
	 */
	Frequency
	sample_rate() const noexcept;

	/**
	 * Sets sample rate and updates all connected port buffers.
	 */
	void
	set_sample_rate (Frequency sample_rate);

	/**
	 * Returns current graph tempo measured
	 * in quarter notes per minute (BPM).
	 */
	Frequency
	tempo() const noexcept;

	/**
	 * Sets new tempo.
	 */
	void
	set_tempo (Frequency tempo);

	/**
	 * Returns current master tune in Hz. Default value is 440.0.
	 */
	Frequency
	master_tune() const noexcept;

	/**
	 * Sets new master tune in Hz.
	 */
	void
	set_master_tune (Frequency master_tune);

	/**
	 * Sends notification to all registered units
	 * and then deletes it.
	 */
	void
	notify (Unique<Notification>);

	/**
	 * Returns timestamp of last entering into processing round.
	 */
	Time
	timestamp() const noexcept;

	/**
	 * Gives access to set containing all registered Synces (in this graph):
	 */
	Units const&
	units() const noexcept;

	/**
	 * Returns true if syncing is dummy.
	 */
	bool
	dummy() const noexcept;

	/**
	 * Returns number of samples from the beginning of current processing round
	 * to the next tempo tick.
	 */
	uint64_t
	next_tempo_tick() const noexcept;

  private:
	void
	compute_next_tempo_tick();

  public:
	// Signals.
	// It is not defined from within what thread these signals will be emitted.
	Signal::Emiter<Unit*>			unit_registered;
	Signal::Emiter<Unit*>			unit_unregistered;
	Signal::Emiter<Unit*>			unit_retitled;
	// port_renamed is called also when port's comment is changed:
	Signal::Emiter<Port*>			port_renamed;
	Signal::Emiter<Port*, Port*>	port_connected_to;
	Signal::Emiter<Port*, Port*>	port_disconnected_from;
	Signal::Emiter<Port*, Unit*>	port_registered;
	Signal::Emiter<Port*, Unit*>	port_unregistered;
	Signal::Emiter<PortGroup*>		port_group_renamed;

  private:
	// Set of all registered units:
	Units			_units;

	// True between calls of start_/finish_processing_round:
	bool			_inside_processing_round	= false;
	bool			_dummy_syncing				= false;

	// Timestamp of last enter_processing_round:
	Time			_timestamp;
	unsigned int	_next_tempo_tick			= 0;

	// Graph parameters:
	std::size_t		_buffer_size				= 0;
	Frequency		_sample_rate				= 0_Hz;
	Frequency		_tempo						= 120_Hz / 60;
	Frequency		_master_tune				= 440_Hz;

	// Registered backends:
	AudioBackend*	_audio_backend				= nullptr;
	EventBackend*	_event_backend				= nullptr;
};


inline AudioBackend*
Graph::audio_backend() const noexcept
{
	return _audio_backend;
}


inline EventBackend*
Graph::event_backend() const noexcept
{
	return _event_backend;
}


inline std::size_t
Graph::buffer_size() const noexcept
{
	return _buffer_size;
}


inline Frequency
Graph::sample_rate() const noexcept
{
	return _sample_rate;
}


inline Frequency
Graph::tempo() const noexcept
{
	return _tempo;
}


inline Frequency
Graph::master_tune() const noexcept
{
	return _master_tune;
}


inline Time
Graph::timestamp() const noexcept
{
	return _timestamp;
}


inline Graph::Units const&
Graph::units() const noexcept
{
	return _units;
}


inline bool
Graph::dummy() const noexcept
{
	return _dummy_syncing;
}


inline uint64_t
Graph::next_tempo_tick() const noexcept
{
	return _next_tempo_tick;
}

} // namespace Haruhi

#endif

