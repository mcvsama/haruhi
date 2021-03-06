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
#include <haruhi/config/all.h>
#include <haruhi/lib/midi.h>

// Local:
#include "controller_with_port_item.h"
#include "backend.h"


namespace Haruhi {

namespace EventBackendImpl {

ControllerWithPortItem::ControllerWithPortItem (DeviceWithPortItem* parent, DevicesManager::Controller* controller):
	ControllerItem (parent, controller),
	PortItem (parent->backend()),
	_device_item (parent)
{
	// Allocate new port:
	backend()->graph()->synchronize ([&] {
		_port = std::make_unique<EventPort> (backend(), controller->name().toStdString(), Port::Output, parent->port_group());
		_device_item->controllers()->insert (this);
	});
	// Fully constructed:
	set_ready (true);
}


ControllerWithPortItem::~ControllerWithPortItem()
{
	Mutex::Lock lock (*backend()->graph());
	_device_item->controllers()->erase (this);
	_port.reset();
}


void
ControllerWithPortItem::update_name()
{
	_port->set_name (name().toStdString());
}


bool
ControllerWithPortItem::handle_event (MIDI::Event const& midi_event, DeviceWithPortItem& device_item)
{
	if (learning())
		learn_from_event (midi_event);
	return _controller->handle_event (midi_event, *device_item.device(), *_port->buffer(), _port->graph());
}


void
ControllerWithPortItem::generate_smoothing_events()
{
	_controller->generate_smoothing_events (*_port->buffer(), _port->graph());
}

} // namespace EventBackendImpl

} // namespace Haruhi

