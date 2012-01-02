/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <map>

// Haruhi:
#include <haruhi/graph/event.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "voice_manager.h"
#include "event_dispatcher.h"
#include "params.h"


namespace MikuruPrivate {

void
EventDispatcher::VoiceParamReceiver::receive (Haruhi::VoiceID voice_id, int value)
{
	_voice_manager->set_voice_param (voice_id, _voice_param, value);
}


void
EventDispatcher::VoiceFilterParamReceiver::receive (Haruhi::VoiceID voice_id, int value)
{
	switch (_filter_id)
	{
		case Filter1:
			_voice_manager->set_filter1_param (voice_id, _filter_param, value);
			break;

		case Filter2:
			_voice_manager->set_filter2_param (voice_id, _filter_param, value);
			break;
	}
}


EventDispatcher::EventDispatcher (Haruhi::EventPort* port, Haruhi::Knob* knob, Receiver* receiver):
	_port (port),
	_knob (knob),
	_receiver (receiver),
	_min (0),
	_max (0)
{
}


EventDispatcher::EventDispatcher (Haruhi::EventPort* port, int min, int max, Receiver* receiver):
	_port (port),
	_knob (0),
	_receiver (receiver),
	_min (min),
	_max (max)
{
}


void
EventDispatcher::process_events()
{
	Haruhi::EventBuffer* buffer = _port->event_buffer();
	Haruhi::ControllerEvent const* ce = 0;
	Haruhi::VoiceControllerEvent const* vce = 0;

	_vcemap.clear();

	if (!buffer->events().empty())
	{
		// Use last controller value:
		for (Haruhi::EventBuffer::Events::const_iterator e = buffer->events().begin(); e != buffer->events().end(); ++e)
		{
			switch ((*e)->event_type())
			{
				case Haruhi::Event::ControllerEventType:
					ce = static_cast<Haruhi::ControllerEvent const*> (e->get());
					break;

				case Haruhi::Event::VoiceControllerEventType:
					vce = static_cast<Haruhi::VoiceControllerEvent const*> (e->get());
					_vcemap[vce->voice_id()] = vce;
					break;

				default:
					break;
			}
		}
	}

	if (ce)
	{
		if (_knob)
			_knob->process_event (ce);
		else
			_receiver->receive (Haruhi::OmniVoice, renormalize (ce->value(), 0.0f, 1.0f, _min, _max));
	}

	if (_knob)
	{
		for (VCEMap::iterator v = _vcemap.begin(); v != _vcemap.end(); ++v)
		{
			float const val = _knob->controller_proxy().config().forward_normalized (v->second->value());
			_receiver->receive (v->second->voice_id(), val);
		}
	}
	else
	{
		for (VCEMap::iterator v = _vcemap.begin(); v != _vcemap.end(); ++v)
			_receiver->receive (v->second->voice_id(), renormalize (v->second->value(), 0.0f, 1.0f, _min, _max));
	}
}

} // namespace MikuruPrivate

