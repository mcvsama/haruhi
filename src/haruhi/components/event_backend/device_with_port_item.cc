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

// Local:
#include "device_with_port_item.h"
#include "controller_with_port_item.h"
#include "backend.h"


namespace Haruhi {

namespace EventBackendImpl {

DeviceWithPortItem::DeviceWithPortItem (Backend* p_backend, Tree* parent, DevicesManager::Device* device):
	DeviceItem (parent, device),
	PortItem (p_backend)
{
	_transport_port = backend()->transport()->create_input (device->name().toStdString());
	backend()->_inputs[_transport_port] = this;
	// Allocate port group:
	backend()->graph()->lock();
	_port_group = new PortGroup (backend()->graph(), device->name().toStdString());
	backend()->graph()->unlock();
	// Ready for handling events:
	set_ready (true);
}


DeviceWithPortItem::~DeviceWithPortItem()
{
	// Delete children:
	QList<QTreeWidgetItem*> children = takeChildren();
	while (!children.isEmpty())
		delete children.takeFirst();
	// TODO lock for _inputs map:
	backend()->_inputs.erase (_transport_port);
	backend()->transport()->destroy_port (_transport_port);
	backend()->graph()->lock();
	delete _port_group;
	backend()->graph()->unlock();
}


ControllerItem*
DeviceWithPortItem::create_controller_item (DevicesManager::Controller* controller)
{
	return new ControllerWithPortItem (this, controller);
}


void
DeviceWithPortItem::update_name()
{
	_transport_port->rename (name().toStdString());
	// Update group name:
	backend()->graph()->lock();
	_port_group->set_name (name().toStdString());
	backend()->graph()->unlock();
}

} // namespace EventBackendImpl

} // namespace Haruhi

