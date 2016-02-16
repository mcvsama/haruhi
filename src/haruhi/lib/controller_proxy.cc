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
	auto buffer = _event_port->buffer();

	if (!buffer->events().empty())
	{
		for (auto& event: buffer->events())
		{
			switch (event->event_type())
			{
				case Event::ControllerEventType:
				{
					auto ce = static_cast<ControllerEvent const*> (event.get());
					process_event (ce);
					break;
				}

				case Event::VoiceControllerEventType:
				{
					auto vce = static_cast<VoiceControllerEvent const*> (event.get());
					on_voice_controller_event (vce, param()->adapter()->forward_normalized (vce->value()));
					break;
				}

				default:
					break;
			}
		}
	}
}

} // namespace Haruhi
