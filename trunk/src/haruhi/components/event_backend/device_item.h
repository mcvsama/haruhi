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

// Local:
#include "event_transport.h"
#include "port_item.h"
#include "ports_list_view.h"


namespace Haruhi {

namespace EventBackendPrivate {

class DeviceItem: public PortItem
{
	friend class PortsListView;

  public:
	DeviceItem (PortsListView* parent, QString const& name);

	virtual ~DeviceItem();

	void
	update_name();

	QString
	name() const;

	EventTransport::Port*
	transport_port() const { return _transport_port; }

	Core::PortGroup*
	port_group() const;

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  private:
	Core::PortGroup*		_port_group;
	EventTransport::Port*	_transport_port;
};

} // namespace EventBackendPrivate

} // namespace Haruhi

#endif

