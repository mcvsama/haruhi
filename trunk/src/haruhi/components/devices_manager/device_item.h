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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__DEVICE_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__DEVICE_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/graph/port_group.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "item.h"
#include "tree.h"
#include "controller.h"


namespace Haruhi {

namespace DevicesManager {

class Device;
class ControllerItem;

class DeviceItem: public Item
{
  public:
	/**
	 * \param	device Device that will be associated with this UI item.
	 */
	DeviceItem (Tree* parent, Device* device);

	virtual ~DeviceItem();

	/**
	 * Returns device name used in Haruhi.
	 */
	QString
	name() const { return _device->name(); }

	/**
	 * Sets new name for item and device.
	 */
	void
	set_name (QString const& name);

	/**
	 * Allocates ControllerItem that will be used as child for this DeviceItem.
	 * \param	controller Controller object associated with newly created item.
	 * 			Item will not take ownership of the object.
	 */
	virtual ControllerItem*
	create_controller_item (Controller* controller);

	/**
	 * Returns Device object associated with this UI item.
	 */
	Device*
	device() const { return _device; }

  private:
	// Device associated with this UI item; not owned:
	Device*	_device;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

