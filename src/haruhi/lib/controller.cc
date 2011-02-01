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
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/session/session.h>
#include <haruhi/session/unit_bay.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "controller.h"


namespace Haruhi {

Controller::Controller (EventPort* event_port, ControllerParam* controller_param):
	_controller_proxy (event_port, controller_param),
	_unit_bay (0),
	_learning (false)
{
	if (event_port)
		event_port->learned_connection_signal.connect (this, &Controller::learned_connection);
	_controller_proxy.set_widget (this);
	schedule_for_update();
}


Controller::~Controller()
{
	Signal::Receiver::disconnect_all_signals();
	// Ensure that ControllerProxy will not request periodic-update
	// on this widget anymore:
	_controller_proxy.set_widget (0);
	// Forget current periodic-update request:
	forget_about_update();
}


void
Controller::start_learning()
{
	EventPort* port = _controller_proxy.event_port();
	if (port)
	{
		port->start_learning (EventBackend::Controller | EventBackend::Pitchbend);
		_learning.store (true);
		learning_state_changed();
	}
}


void
Controller::stop_learning()
{
	EventPort* port = _controller_proxy.event_port();
	if (port)
	{
		port->stop_learning();
		_learning.store (false);
		learning_state_changed();
	}
}


void
Controller::learned_connection (EventBackend::EventTypes, EventPort* learned_port)
{
	_learning.store (false);
	// We're callback from engine thread, not UI thread, so use safe update method:
	schedule_for_update();
}

} // namespace Haruhi