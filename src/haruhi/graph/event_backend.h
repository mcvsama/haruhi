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

#ifndef HARUHI__GRAPH__EVENT_BACKEND_H__INCLUDED
#define HARUHI__GRAPH__EVENT_BACKEND_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>

// Haruhi:
#include <haruhi/config/types.h>

// Local:
#include "backend.h"


namespace Haruhi {

class EventPort;

class EventTeacher
{
  public:
	// AudioBackend has always fixed ID:
	enum { ID = 0x10001 };

	/**
	 * Ports to listen for in start_learning()/stop_learning().
	 * As transport type use EventTypes.
	 */
	enum {
		Keyboard			= 1 << 0,
		Controller			= 1 << 1,
		Pitchbend			= 1 << 2,
		ChannelPressure		= 1 << 3,
		KeyPressure			= 1 << 4,
	};

	typedef uint32_t EventTypes;

	/**
	 * Objects deriving this class can use EventBackend
	 * to automatically learn which event port to use
	 * for eg. given knob.
	 *
	 * Object of this class must ensure that it's not registered in EventTeacher
	 * for learning when it's destructed. Otherwise it will result in segfaults.
	 *
	 * See EventTeacher::start_learning() and EventTeacher::stop_learning() methods.
	 */
	class Learnable
	{
	  public:
		/**
		 * Will be called from within engine thread when port is learned.
		 * \param	event_types Original mask of event types passed to EventTeacher::start_learning().
		 * \entry	engine thread
		 */
		virtual void
		learned_connection (EventTypes event_types, EventPort* event_port) = 0;
	};

  protected:
	typedef std::map<Learnable*, EventTypes> Learnables;

  public:
	~EventTeacher();

	/**
	 * Starts listening for specified event types.
	 * When specified event arrive on any input port Learnable
	 * is notified about it.
	 */
	void
	start_learning (Learnable*, EventTypes);

	/**
	 * Stops learning started with start_learning(),
	 * eg. when user changes his mind.
	 */
	void
	stop_learning (Learnable*);

  protected:
	/**
	 * Returns set of objects that are currently in 'learn'
	 * state, wanting to be notified about learned connections.
	 */
	Learnables&
	learnables() { return _learnables; }

  private:
	Learnables _learnables;
};


class EventBackend:
	public Backend,
	public EventTeacher
{
  public:
	EventBackend (std::string const& title);
};

} // namespace Haruhi

#endif

