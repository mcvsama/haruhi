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

// Haruhi:
#include <haruhi/core/event.h>
#include <haruhi/core/event_port.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "controller_proxy.h"


ControllerProxy::Config::Config (int limit_min, int limit_max):
	curve (0.0f),
	hard_limit_min (limit_min),
	hard_limit_max (limit_max),
	user_limit_min (limit_min),
	user_limit_max (limit_max)
{
}


int
ControllerProxy::Config::forward (int in) const
{
	return renormalize (encurve (in), hard_limit_min, hard_limit_max, user_limit_min, user_limit_max);
}


int
ControllerProxy::Config::reverse (int in) const
{
	return decurve (renormalize (in, user_limit_min, user_limit_max, hard_limit_min, hard_limit_max));
}


int
ControllerProxy::Config::encurve (int in) const
{
	float power = curve < 0
		? renormalize (curve, -1.0, 0.0, 0.4, 1.0)
		: renormalize (curve, 0.0, +1.0, 1.0, 2.5);
	return renormalize (std::pow (std::max (+0.0f, renormalize (in, 1.0 * hard_limit_min, 1.0 * hard_limit_max, 0.0, 1.0)), power),
						0.0, 1.0, 1.0 * hard_limit_min, 1.0 * hard_limit_max);
}


int
ControllerProxy::Config::decurve (int in) const
{
	float power = curve < 0
		? renormalize (curve, -1.0, 0.0, 0.4, 1.0)
		: renormalize (curve, 0.0, +1.0, 1.0, 2.5);
	return renormalize (std::pow (renormalize (in, 1.0 * hard_limit_min, 1.0 * hard_limit_max, 0.0, 1.0), 1.0 / power),
						0.0, 1.0, 1.0 * hard_limit_min, 1.0 * hard_limit_max);
}


ControllerProxy::ControllerProxy (Core::EventPort* event_port, int volatile* parameter, int volatile* smoothing_parameter, int limit_min, int limit_max, int value):
	_config (limit_min, limit_max),
	_default_value (value),
	_parameter (parameter),
	_smoothing_parameter (smoothing_parameter),
	_event_port (event_port),
	_widget (0)
{
	set_value (value);
}


void
ControllerProxy::apply_config()
{
	if (_widget)
		_widget->schedule_for_update();
}


void
ControllerProxy::process_events()
{
	Core::EventBuffer* buffer = _event_port->event_buffer();

	if (!buffer->events().empty())
	{
		// Use last controller value from buffer:
		for (Core::EventBuffer::EventsMultiset::reverse_iterator e = buffer->events().rbegin(); e != buffer->events().rend(); ++e)
		{
			if (e != buffer->events().rend())
			{
				if ((*e)->event_type() == Core::Event::ControllerEventType)
				{
					process_event (static_cast<Core::ControllerEvent const*> (e->get()));
					break;
				}
			}
		}
	}
}


void
ControllerProxy::process_event (Core::ControllerEvent const* event)
{
	// Update parameter:
	atomic (*_parameter) = _config.forward (renormalize (event->value(), 0.0f, 1.0f, 1.0f * _config.hard_limit_min, 1.0f * _config.hard_limit_max));
	// Schedule update for paired Widget:
	if (_widget)
		_widget->schedule_for_update();
}


void
ControllerProxy::save_state (QDomElement& element) const
{
	element.setAttribute ("curve", QString ("%1").arg (_config.curve, 0, 'f', 1));
	element.setAttribute ("user-limit-min", QString ("%1").arg (_config.user_limit_min));
	element.setAttribute ("user-limit-max", QString ("%1").arg (_config.user_limit_max));
}


void
ControllerProxy::load_state (QDomElement const& element)
{
	_config.curve = bound (element.attribute ("curve").toFloat(), -1.0f, 1.0f);
	_config.user_limit_min = bound (element.attribute ("user-limit-min").toInt(), _config.hard_limit_min, _config.hard_limit_max);
	_config.user_limit_max = bound (element.attribute ("user-limit-max").toInt(), _config.hard_limit_min, _config.hard_limit_max);
	apply_config();
}

