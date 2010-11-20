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
	_controller_proxy.set_widget (this);
	schedule_for_update();
}


Controller::~Controller()
{
	// Ensure that engine thread will not notify us about
	// learned port:
	if (unit_bay())
		stop_learning();
	// Ensure that ControllerProxy will not request periodic-update
	// on this widget anymore:
	_controller_proxy.set_widget (0);
	// Forget current periodic-update request:
	forget_about_update();
}


void
Controller::start_learning()
{
	_learning.store (true);
	unit_bay()->graph()->event_backend()->start_learning (this, EventBackend::Controller | EventBackend::Pitchbend);
	learning_state_changed();
}


void
Controller::stop_learning()
{
	_learning.store (false);
	unit_bay()->graph()->event_backend()->stop_learning (this, EventBackend::Controller | EventBackend::Pitchbend);
	learning_state_changed();
}


void
Controller::learned_port (EventBackend::EventTypes, EventPort* event_port)
{
	if (unit_bay())
	{
		_learning.store (false);
		unit_bay()->graph()->lock();
		event_port->connect_to (controller_proxy().event_port());
		unit_bay()->graph()->unlock();
		// We're in engine thread, not UI thread, so use safe update method:
		schedule_for_update();
	}
}

} // namespace Haruhi
