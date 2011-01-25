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

// Qt:
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "device.h"
#include "device_item.h"
#include "controller_item.h"


namespace Haruhi {

namespace DevicesManager {

DeviceItem::DeviceItem (Tree* parent, Device* device):
	Item (parent, device->name()),
	_device (device)
{
	// Configure item:
	setIcon (0, Resources::Icons16::keyboard());
	update_minimum_size();
}


DeviceItem::~DeviceItem()
{
	// Delete children:
	while (childCount() > 0)
	{
		QTreeWidgetItem* c = child (0);
		takeChild (0);
		delete c;
	}
	// Remove itself from External ports list view:
	if (treeWidget())
		treeWidget()->invisibleRootItem()->takeChild (treeWidget()->invisibleRootItem()->indexOfChild (this));
}


void
DeviceItem::set_name (QString const& name)
{
	setText (0, name);
	_device->set_name (name);
}


ControllerItem*
DeviceItem::create_controller_item (Controller* controller)
{
	return new ControllerItem (this, controller);
}

} // namespace DevicesManager

} // namespace Haruhi

