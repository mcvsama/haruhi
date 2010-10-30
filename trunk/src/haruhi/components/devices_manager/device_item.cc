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
#include "controller_item.h"


namespace Haruhi {

namespace DevicesManagerPrivate {

DeviceItem::DeviceItem (PortsListView* parent, QString const& name):
	Item (parent, name)
{
	// Configure item:
	setIcon (0, Config::Icons16::keyboard());
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


ControllerItem*
DeviceItem::create_controller_item (DeviceItem* parent, QString const& name)
{
	return new ControllerItem (parent, name);
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
			QDomElement e = element.ownerDocument().createElement ("controller");
			controller_item->save_state (e);
			element.appendChild (e);
		}
	}
}


void
DeviceItem::load_state (QDomElement const& element)
{
	setText (0, element.attribute ("name"));
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "controller")
			{
				ControllerItem* port = create_controller_item (this, e.attribute ("name"));
				port->load_state (e);
			}
		}
	}
}

} // namespace DevicesManagerPrivate

} // namespace Haruhi

