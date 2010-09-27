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
#include <haruhi/unit_bay.h>
#include <haruhi/session.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "controller.h"


namespace Haruhi {

Controller::Controller (ControllerProxy* controller_proxy):
	_controller_proxy (controller_proxy),
	_unit_bay (0),
	_learning (false)
{ }


Controller::~Controller()
{ }


void
Controller::start_learning()
{
	atomic (_learning) = true;
	unit_bay()->session()->event_backend()->start_learning (this, EventBackend::Controller | EventBackend::Pitchbend);
	learning_state_changed();
}


void
Controller::stop_learning()
{
	atomic (_learning) = false;
	unit_bay()->session()->event_backend()->stop_learning (this, EventBackend::Controller | EventBackend::Pitchbend);
	learning_state_changed();
}


void
Controller::learned_port (EventBackend::EventTypes, Core::EventPort* event_port)
{
	if (unit_bay())
	{
		atomic (_learning) = false;
		unit_bay()->graph()->lock();
		event_port->connect_to (controller_proxy()->event_port());
		unit_bay()->graph()->unlock();
		// We're in engine thread, not UI thread, so use safe update method:
		schedule_for_update();
	}
}

} // namespace Haruhi
