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
#include "exception.h"
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
	audio_backend() const { return _audio_backend; }

	/**
	 * Returns currently registered event backend (or 0 if there's none).
	 */
	EventBackend*
	event_backend() const { return _event_backend; }

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
	buffer_size() const { return _buffer_size; }

	/**
	 * Sets buffer size and updates all connected port buffers.
	 */
	void
	set_buffer_size (std::size_t buffer_size);

	/**
	 * Returns current sample rate.
	 */
	std::size_t
	sample_rate() const { return _sample_rate; }

	/**
	 * Sets sample rate and updates all connected port buffers.
	 */
	void
	set_sample_rate (std::size_t sample_rate);

	/**
	 * Returns current graph tempo.
	 */
	float
	tempo() const { return _tempo; }

	/**
	 * Sets new tempo.
	 */
	void
	set_tempo (float tempo);

	/**
	 * Returns current master tune in Hz. Default value is 440.0.
	 */
	float
	master_tune() const { return _master_tune; }

	/**
	 * Sets new master tune in Hz.
	 */
	void
	set_master_tune (float master_tune);

	/**
	 * Sends notification to all registered units
	 * and then deletes it.
	 */
	void
	notify (Notification* notification);

	/**
	 * Returns timestamp of last entering into processing round.
	 */
	Timestamp
	timestamp() const { return _timestamp; }

	/**
	 * Gives access to set containing all registered Synces (in this graph):
	 */
	Units const&
	units() const;

	/**
	 * Creates timestamp at the time of call.
	 */
	static Timestamp
	now();

	/**
	 * Returns true if syncing is dummy.
	 */
	bool
	dummy() const { return _dummy_syncing; }

  public:
	// Signals:
	Signal::Emiter1<Unit*>			unit_registered;
	Signal::Emiter1<Unit*>			unit_unregistered;
	Signal::Emiter1<Unit*>			unit_retitled;
	// Called also when port's comment is changed:
	Signal::Emiter1<Port*>			port_renamed;
	Signal::Emiter2<Port*, Port*>	port_connected_to;
	Signal::Emiter2<Port*, Port*>	port_disconnected_from;
	Signal::Emiter2<Port*, Unit*>	port_registered;
	Signal::Emiter2<Port*, Unit*>	port_unregistered;
	Signal::Emiter1<PortGroup*>		port_group_renamed;

  private:
	// Set of all registered synces:
	Units			_units;

	// True between calls of start_/finish_processing_round:
	bool			_inside_processing_round;
	bool			_dummy_syncing;

	// Timestamp of last enter_processing_round:
	Timestamp		_timestamp;

	// Graph parameters:
	std::size_t		_buffer_size;
	std::size_t		_sample_rate;
	float			_tempo;
	float			_master_tune;

	// Registered backends:
	AudioBackend*	_audio_backend;
	EventBackend*	_event_backend;
};

} // namespace Haruhi

#endif

