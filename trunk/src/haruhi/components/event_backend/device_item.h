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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__DEVICE_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__DEVICE_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/core/port_group.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "event_transport.h"
#include "port_item.h"
#include "ports_list_view.h"


namespace Haruhi {

namespace EventBackendPrivate {

class DeviceItem:
	public Item,
	public SaveableState
{
	friend class PortsListView;

  public:
	DeviceItem (PortsListView* parent, QString const& name);

	virtual ~DeviceItem();

	QString
	name() const { return QTreeWidgetItem::text (0); }

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);
};


class ControllerWithPortItem;

class DeviceWithPortItem:
	public DeviceItem,
	public PortItem
{
	friend class PortsListView;

  public:
	typedef std::set<ControllerWithPortItem*> Controllers;

  public:
	DeviceWithPortItem (EventBackend* backend, PortsListView* parent, QString const& name);

	virtual ~DeviceWithPortItem();

	Controllers*
	controllers() { return &_controllers; }

	EventTransport::Port*
	transport_port() const { return _transport_port; }

	Core::PortGroup*
	port_group() const { return _port_group; }

	void
	update_name();

	void
	load_state (QDomElement const&);

  protected:
	// For quick traversal over children. Child items
	// will add/remove itself from this set:
	Controllers _controllers;

  private:
	Core::PortGroup*		_port_group;
	EventTransport::Port*	_transport_port;
};

} // namespace EventBackendPrivate

} // namespace Haruhi

#endif

