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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/graph/event.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "controller_proxy.h"


namespace Haruhi {

ControllerProxy::ControllerProxy (EventPort* event_port, ControllerParam* param):
	_param (param),
	_event_port (event_port),
	_widget (0)
{
	// Follow parameter changes to update widget:
	_param->on_change.connect (this, &ControllerProxy::param_changed);
}


ControllerProxy::~ControllerProxy()
{
	_param->on_change.disconnect (this, &ControllerProxy::param_changed);
}


void
ControllerProxy::process_events()
{
	EventBuffer* buffer = _event_port->event_buffer();

	if (!buffer->events().empty())
	{
		bool found_ce = false;
		bool found_vce = false;

		// Use last controller value from buffer:
		for (EventBuffer::Events::const_reverse_iterator e = buffer->events().rbegin(); e != buffer->events().rend() && !(found_ce && found_vce); ++e)
		{
			switch ((*e)->event_type())
			{
				case Event::ControllerEventType:
					if (!found_ce)
					{
						ControllerEvent const* ce = static_cast<ControllerEvent const*> (e->get());
						process_event (ce);
						found_ce = true;
					}
					break;

				case Event::VoiceControllerEventType:
					if (!found_vce)
					{
						VoiceControllerEvent const* vce = static_cast<VoiceControllerEvent const*> (e->get());
						on_voice_controller_event (vce, param()->adapter()->forward_normalized (vce->value()));
						found_vce = true;
					}
					break;

				default:
					break;
			}
		}
	}
}

} // namespace Haruhi
