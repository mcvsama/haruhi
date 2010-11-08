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

#ifndef HARUHI__CORE__UNIT_H__INCLUDED
#define HARUHI__CORE__UNIT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <set>

// Haruhi:
#include <haruhi/utility/noncopyable.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/mutex.h>

// Local:
#include "port.h"
#include "graph.h"
#include "buffer.h"
#include "exception.h"
#include "notification.h"


namespace Haruhi {

class Session;

namespace Core {

/*
 * Implements object that has two states: synced, and ready to sync.
 * Primary object task should be processing/generating data
 * (typically audio/event data) in each audio processing round.
 * When data is generated, object goes into synced state and then
 * cannot generate new data, until it is waken up.
 * Unit objects will be waken up at beginning of each processing round.
 */
class Unit: private Noncopyable
{
	friend class Graph;
	friend class Port;

  public:
	// Shorthand types:
	typedef std::set<Unit*>  Set;

	class ComparePointerByTitle
	{
	  public:
		bool
		operator() (Unit* a, Unit* b) { return a->title() < b->title(); }
	};

  public:
	/**
	 * Creates Unit.
	 * \param	session is user data. No special meaning.
	 * \param	urn identifies Unit type. Used by notification system.
	 * \param	title is Unit's name visible to user.
	 * \param	id is Unit ID. It's not used internally by Unit, but might be useful
	 * 			for other objects. It's not verified to be unique. If 0, then new ID
	 * 			will be automatically assigned.
	 */
	Unit (Session* session, std::string const& urn, std::string const& title, int id = 0);

	/**
	 * Dtor
	 * Unit must unregister all of its ports before destruction.
	 * Unit must be disabled before destruction.
	 */
	virtual ~Unit();

	/**
	 * Synchronizes unit - calls its process() method.
	 */
	void
	sync();

	/**
	 * Returns pointer to associated session.
	 * Used as 'user data', Session doesn't have any special meaning
	 * to Unit.
	 */
	Session*
	session() const { return _session; }

	/**
	 * Returns Graph that owns this Unit.
	 */
	Graph*
	graph() const { return _graph; }

	/**
	 * Called after unit has been registered to new graph.
	 * Graph is not locked when executing this method.
	 */
	virtual void
	registered() { }

	/**
	 * Called before unit is unregistered.
	 * Graph is not locked when executing this method.
	 * Unit is disabled before entering this method.
	 */
	virtual void
	unregistered() { }

	/**
	 * Receives Notifications. Default implementation
	 * does nothing.
	 */
	virtual void
	notify (Notification*) { }

	/**
	 * Enables unit. Enabled unit are synced periodically by Graph.
	 * May not be called inside of processing round.
	 */
	void
	enable() { atomic (_enabled) = true; }

	/**
	 * Disables unit. Disabled units aren't synced.
	 * May not be called inside of processing round.
	 */
	void
	disable();

	/**
	 * Tells whether unit is enabled. May not be called inside of processing round.
	 */
	bool
	enabled() const { return atomic (_enabled); }

	/**
	 * Returns Unit's URN that identifies unit type.
	 * May not be called inside of processing round.
	 */
	std::string
	urn() const { return _urn; }

	/**
	 * Returns Unit's title to be presented to the user.
	 * May not be called inside of processing round.
	 */
	std::string
	title() const { return _title; }

	/**
	 * Returns Unit's id.
	 * May not be called inside of processing round.
	 */
	int
	id() const { return _id; }

	/**
	 * Sets unit id.
	 * May not be called inside of processing round.
	 */
	void
	set_id (int id);

	/**
	 * Sets new title for unit and triggers notification in graph.
	 * May not be called inside of processing round.
	 */
	void
	set_title (std::string const& title);

	/**
	 * Returns input ports list.
	 */
	Ports const&
	inputs() const;

	/**
	 * Returns output ports list.
	 */
	Ports const&
	outputs() const;

	/**
	 * Allocates and returns unique ID for new unit.
	 */
	static int
	allocate_id();

  protected: // By Chuck Norris
	/**
	 * Synchronizes all connected inputs.
	 * Any input must be synchronized before accessing it's buffers.
	 * Multiple synchronizations are allowed (unit will synchronize
	 * only once).
	 *
	 * May be only called inside of processing round.
	 */
	void
	sync_inputs()
	{
		for (Ports::const_iterator i = _inputs.begin();  i != _inputs.end();  ++i)
			(*i)->sync();
	}

	/**
	 * Clears (prepares) buffers of all output ports.
	 */
	void
	clear_outputs()
	{
		for (Ports::const_iterator o = _outputs.begin();  o != _outputs.end();  ++o)
			(*o)->buffer()->clear();
	}

	/**
	 * This method should synchronize Unit, that is:
	 * prepare output buffers for all of its output ports.
	 */
	virtual void
	process() = 0;

	/**
	 * Tells unit to reset (stop any sounds it generates).
	 * Default implementation does nothing.
	 * Beware, that it might be called also inside of processing round.
	 */
	virtual void
	panic();

	/**
	 * Called after change of graph parameters: buffer size, tempo or sample rate.
	 * Updates ports buffer sizes, etc.
	 * You may override this method in subclass, but ensure that this implementation
	 * is also called and beware that it might be called also inside of processing round
	 * (use graph()->lock/unlock()).
	 */
	virtual void
	graph_updated();

  private:
	static int	_id_counter;

	Session*	_session;
	Graph*		_graph;
	bool		_synced;
	bool		_enabled;
	// Used by disable() and sync() methods:
	Mutex		_processing_mutex;

	std::string _urn;
	std::string	_title;

	// IDs are not checked to be unique.
	int			_id;

	Ports		_inputs;
	Ports		_outputs;
};

} // namespace Core

} // namespace Haruhi

#endif

