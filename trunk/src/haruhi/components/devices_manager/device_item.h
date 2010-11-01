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
#include <haruhi/core/port_group.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "item.h"
#include "ports_list_view.h"


namespace Haruhi {

namespace DevicesManager {

class ControllerItem;

class DeviceItem:
	public Item,
	public SaveableState
{
	friend class PortsListView;

  public:
	DeviceItem (PortsListView* parent, QString const& name);

	virtual ~DeviceItem();

	/**
	 * Returns device name used in Haruhi.
	 */
	QString
	name() const { return QTreeWidgetItem::text (0); }

	/**
	 * Allocates ControllerItem that will be used
	 * as child for this DeviceItem.
	 */
	virtual ControllerItem*
	create_controller_item (QString const& name);

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

