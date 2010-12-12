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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__PORTS_LIST_VIEW_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__PORTS_LIST_VIEW_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidget>

// Haruhi:
#include <haruhi/utility/saveable_state.h>


namespace Haruhi {

namespace DevicesManager {

class DeviceItem;

class PortsListView:
	public QTreeWidget,
	public SaveableState
{
  public:
	PortsListView (QWidget* parent);

	/**
	 * Allocates DeviceItem that will be used
	 * as child for this PortsList.
	 */
	virtual DeviceItem*
	create_device_item (QString const& name);

	QTreeWidgetItem*
	selected_item() const;

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

	void
	load_devices_from_settings();

	void
	save_devices_to_settings();
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

