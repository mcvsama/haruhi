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
#include <map>

// Haruhi:
#include <haruhi/core/event.h>
#include <haruhi/core/event_port.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "voice_manager.h"
#include "event_dispatcher.h"
#include "params.h"


namespace MikuruPrivate {

void
EventDispatcher::VoiceParamReceiver::receive (Core::VoiceID voice_id, int value)
{
	_voice_manager->set_voice_param (voice_id, _voice_param, value);
}


void
EventDispatcher::VoiceFilterParamReceiver::receive (Core::VoiceID voice_id, int value)
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


EventDispatcher::EventDispatcher (Core::EventPort* port, Knob* knob, Receiver* receiver):
	_port (port),
	_knob (knob),
	_receiver (receiver),
	_min (knob->controller_proxy()->config()->user_limit_min),
	_max (knob->controller_proxy()->config()->user_limit_max)
{
}


EventDispatcher::EventDispatcher (Core::EventPort* port, int min, int max, Receiver* receiver):
	_port (port),
	_knob (0),
	_receiver (receiver),
	_min (min),
	_max (max)
{
}


void
EventDispatcher::load_events()
{
	Core::EventBuffer* buffer = _port->event_buffer();
	Core::ControllerEvent const* ce = 0;
	Core::VoiceControllerEvent const* vce = 0;

	_vcemap.clear();

	if (!buffer->events().empty())
	{
		// Use last controller value:
		for (Core::EventBuffer::EventsMultiset::const_iterator e = buffer->events().begin(); e != buffer->events().end(); ++e)
		{
			switch ((*e)->event_type())
			{
				case Core::Event::ControllerEventType:
					ce = static_cast<Core::ControllerEvent const*> (e->get());
					break;

				case Core::Event::VoiceControllerEventType:
					vce = static_cast<Core::VoiceControllerEvent const*> (e->get());
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
			_knob->controller_proxy()->process_event (ce);
		else
			_receiver->receive (Core::OmniVoice, renormalize (ce->value(), 0.0f, 1.0f, _min, _max));
	}

	if (_knob)
	{
		for (VCEMap::iterator v = _vcemap.begin(); v != _vcemap.end(); ++v)
		{
			float const val = renormalize (v->second->value(), 0.0f, 1.0f, _knob->controller_proxy()->config()->user_limit_min, _knob->controller_proxy()->config()->user_limit_max);
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

