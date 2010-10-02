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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config.h>

// Local:
#include "device_item.h"
#include "event_backend.h"


namespace Haruhi {

namespace EventBackendPrivate {

DeviceItem::DeviceItem (PortsListView* parent, QString const& name):
	PortItem (parent, name)
{
	_transport_port = _backend->transport()->create_input (name.toStdString());
	// Allocate port group:
	_backend->graph()->lock();
	_port_group = new Core::PortGroup (_backend->graph(), name.ascii());
	_backend->graph()->unlock();
	_backend->_inputs[_transport_port] = this;
	// Configure item:
	setIcon (0, Config::Icons16::keyboard());
	// Fully constructed:
	set_ready (true);
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
	// TODO lock for _inputs map:
	_backend->_inputs.erase (_transport_port);
	_backend->transport()->destroy_port (_transport_port);
	_backend->graph()->lock();
	delete _port_group;
	_backend->graph()->unlock();
	// Remove itself from External ports list view:
	if (treeWidget())
		treeWidget()->invisibleRootItem()->takeChild (treeWidget()->invisibleRootItem()->indexOfChild (this));
}


void
DeviceItem::update_name()
{
	_transport_port->rename (name().toStdString());
	// Update group name:
	_backend->graph()->lock();
	_port_group->set_name (name().toStdString());
	_backend->graph()->unlock();
}


QString
DeviceItem::name() const
{
	return text (0);
}


Core::PortGroup*
DeviceItem::port_group() const
{
	return _port_group;
}


void
DeviceItem::save_state (QDomElement& element) const
{
	element.setAttribute ("name", name());
	for (int i = 0; i < childCount(); ++i)
	{
		ControllerItem* controller_item = dynamic_cast<ControllerItem*> (child (i));
		if (controller_item)
		{
			QDomElement e = element.ownerDocument().createElement ("internal-input");
			controller_item->save_state (e);
			element.appendChild (e);
		}
	}
}


void
DeviceItem::load_state (QDomElement const& element)
{
	setText (0, element.attribute ("name"));
	update_name();
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "internal-input")
			{
				ControllerItem* port = new ControllerItem (this, e.attribute ("name"));
				port->load_state (e);
			}
		}
	}
}

} // namespace EventBackendPrivate

} // namespace Haruhi

