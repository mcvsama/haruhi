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

// Qt:
#include <QtGui/QTreeWidgetItem>

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
	backend()->graph()->synchronize ([&]() {
		_port = new EventPort (backend(), controller->name().toStdString(), Port::Output, parent->port_group());
		_device_item->controllers()->insert (this);
	});
	// Fully constructed:
	set_ready (true);
}


ControllerWithPortItem::~ControllerWithPortItem()
{
	backend()->graph()->synchronize ([&]() {
		_device_item->controllers()->erase (this);
		delete _port;
	});
}


void
ControllerWithPortItem::update_name()
{
	_port->set_name (name().toStdString());
}


bool
ControllerWithPortItem::handle_event (MIDI::Event const& midi_event)
{
	if (learning())
		learn_from_event (midi_event);
	return _controller->handle_event (midi_event, *_port->event_buffer(), _port->graph());
}


void
ControllerWithPortItem::generate_smoothing_events()
{
	_controller->generate_smoothing_events (*_port->event_buffer(), _port->graph());
}

} // namespace EventBackendImpl

} // namespace Haruhi

