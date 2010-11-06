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

// Local:
#include "event_teacher.h"


namespace Haruhi {

namespace EventBackendPrivate {

void
Teacher::handle_event_for_learnables (EventTransport::MidiEvent const& event, Core::EventPort* port)
{
	Learnables::iterator lnext;
	for (Learnables::iterator l = _learnables.begin(); l != _learnables.end(); l = lnext)
	{
		lnext = l;
		++lnext;

		bool learned = false;
		learned |= (l->second & Keyboard) && (event.type == EventTransport::MidiEvent::NoteOn || event.type == EventTransport::MidiEvent::NoteOff);
		learned |= (l->second & Controller) && event.type == EventTransport::MidiEvent::Controller;
		learned |= (l->second & Pitchbend) && event.type == EventTransport::MidiEvent::Pitchbend;
		learned |= (l->second & ChannelPressure) && event.type == EventTransport::MidiEvent::ChannelPressure;
		learned |= (l->second & KeyPressure) && event.type == EventTransport::MidiEvent::KeyPressure;

		if (learned)
		{
			l->first->learned_port (l->second, port);
			_learnables.erase (l);
		}
	}
}


void
Teacher::start_learning (Learnable* learnable, EventTypes event_types)
{
	_learnables.insert (std::make_pair (learnable, event_types));
}


void
Teacher::stop_learning (Learnable* learnable, EventTypes event_types)
{
	_learnables.erase (std::make_pair (learnable, event_types));
}

} // namespace EventBackendPrivate

} // namespace Haruhi

