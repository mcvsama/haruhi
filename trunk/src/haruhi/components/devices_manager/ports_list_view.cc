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
#include <QtGui/QTreeWidget>
#include <QtGui/QHeaderView>

// Local:
#include "ports_list_view.h"
#include "device_item.h"


namespace Haruhi {

namespace DevicesManagerPrivate {

PortsListView::PortsListView (QWidget* parent):
	QTreeWidget (parent)
{
	header()->setClickable (false);
	header()->setResizeMode (QHeaderView::Stretch);
	setMinimumWidth (160);
	setSortingEnabled (true);
	sortByColumn (0, Qt::AscendingOrder);
	setSelectionMode (QTreeWidget::SingleSelection);
	setRootIsDecorated (true);
	setAllColumnsShowFocus (true);
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
	setContextMenuPolicy (Qt::CustomContextMenu);
	setHeaderLabel ("Device templates");
}


QTreeWidgetItem*
PortsListView::selected_item() const
{
	QList<QTreeWidgetItem*> list = selectedItems();
	return list.empty() ? 0 : list.front();
}


DeviceItem*
PortsListView::create_device_item (QString const& name)
{
	return new DeviceItem (this, name);
}


void
PortsListView::save_state (QDomElement& element) const
{
	for (int i = 0; i < invisibleRootItem()->childCount(); ++i)
	{
		DeviceItem* device_item = dynamic_cast<DeviceItem*> (invisibleRootItem()->child (i));
		QDomElement e;

		if (device_item)
		{
			e = element.ownerDocument().createElement ("device");
			device_item->save_state (e);
			element.appendChild (e);
		}
	}
}


void
PortsListView::load_state (QDomElement const& element)
{
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "device")
			{
				DeviceItem* port = create_device_item (e.attribute ("name"));
				port->load_state (e);
			}
		}
	}
}

} // namespace DevicesManagerPrivate

} // namespace Haruhi

