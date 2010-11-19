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

#ifndef HARUHI__GRAPH__EVENT_BACKEND_H__INCLUDED
#define HARUHI__GRAPH__EVENT_BACKEND_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Local:
#include "backend.h"
#include "event_port.h"


namespace Haruhi {

class EventTeacher
{
  public:
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
	 * See start_learning() and stop_learning() methods.
	 */
	class Learnable
	{
	  public:
		/**
		 * Will be called from within engine thread when port is learned.
		 * \entry	engine thread
		 */
		virtual void
		learned_port (EventTypes event_types, EventPort* event_port) = 0;
	};

  protected:
	typedef std::set<std::pair<Learnable*, EventTypes> > Learnables;

  public:
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
	stop_learning (Learnable*, EventTypes);

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
	EventBackend (std::string const& title, int id);
};

} // namespace Haruhi

#endif

