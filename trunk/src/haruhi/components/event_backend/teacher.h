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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__EVENT_TEACHER_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__EVENT_TEACHER_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdint.h>
#include <set>

// Haruhi:
#include <haruhi/core/event_port.h>

// Local:
#include "transport.h"


namespace Haruhi {

namespace EventBackend {

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
	learned_port (EventTypes event_types, Core::EventPort* event_port) = 0;
};


/**
 * Implements 'learning from MIDI' functionality.
 */
class Teacher
{
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
	typedef std::set<std::pair<Learnable*, EventTypes> > Learnables;

  protected:
	void
	handle_event_for_learnables (Transport::MidiEvent const& event, Core::EventPort* port);

  protected:
	Learnables _learnables;
};

} // namespace EventBackend

} // namespace Haruhi

#endif

