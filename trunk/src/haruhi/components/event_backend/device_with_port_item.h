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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__DEVICE_WITH_PORT_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__DEVICE_WITH_PORT_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/graph/port_group.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/components/devices_manager/device_item.h>

// Local:
#include "port_item.h"
#include "ports_list_view.h"
#include "transport.h"


namespace Haruhi {

namespace EventBackend {

using DevicesManager::DeviceItem;
using DevicesManager::ControllerItem;
class ControllerWithPortItem;
class Backend;

class DeviceWithPortItem:
	public DeviceItem,
	public PortItem
{
	friend class PortsListView;

  public:
	typedef std::set<ControllerWithPortItem*> Controllers;

  public:
	DeviceWithPortItem (Backend* backend, PortsListView* parent, QString const& name);

	virtual ~DeviceWithPortItem();

	Controllers*
	controllers() { return &_controllers; }

	Transport::Port*
	transport_port() const { return _transport_port; }

	PortGroup*
	port_group() const { return _port_group; }

	/**
	 * Sets externally visible transport port name to what
	 * was set by user in the UI.
	 */
	void
	update_name();

	/**
	 * Allocates ControllerWithPortItem that will be used
	 * as child for this DeviceWithPortItem.
	 */
	ControllerItem*
	create_controller_item (QString const& name);

	void
	load_state (QDomElement const&);

  protected:
	// For quick traversal over children. Child items
	// will add/remove itself from this set:
	Controllers _controllers;

  private:
	PortGroup*			_port_group;
	Transport::Port*	_transport_port;
};

} // namespace EventBackend

} // namespace Haruhi

#endif

