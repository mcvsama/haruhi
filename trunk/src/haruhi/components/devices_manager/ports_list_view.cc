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

// Haruhi:
#include <haruhi/application/haruhi.h>
#include <haruhi/settings/devices_manager_settings.h>

// Local:
#include "ports_list_view.h"
#include "device_item.h"


namespace Haruhi {

namespace DevicesManager {

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
				DeviceItem* device = create_device_item (e.attribute ("name"));
				device->load_state (e);
			}
		}
	}
}


void
PortsListView::load_devices_from_settings()
{
	clear();
	DevicesManagerSettings* settings = Haruhi::haruhi()->devices_manager_settings();
	for (DevicesManagerSettings::Devices::iterator tpl = settings->devices().begin(); tpl != settings->devices().end(); ++tpl)
	{
		DeviceItem* device = create_device_item (tpl->name());
		device->load_state (tpl->element());
	}
}


void
PortsListView::save_devices_to_settings()
{
	DevicesManagerSettings* settings = Haruhi::haruhi()->devices_manager_settings();
	settings->devices().clear();

	for (int i = 0; i < invisibleRootItem()->childCount(); ++i)
	{
		DeviceItem* device_item = dynamic_cast<DeviceItem*> (invisibleRootItem()->child (i));
		if (device_item)
			settings->save_device (device_item->name(), *device_item);
	}

	settings->save();
}


void
PortsListView::customEvent (QEvent* event)
{
	LearnedParams* lp = dynamic_cast<LearnedParams*> (event);
	if (lp)
	{
		// Force update of editor dialog:
		lp->item->setSelected (false);
		lp->item->setSelected (true);
		// Reset icon:
		lp->item->setIcon (0, Resources::Icons16::event_output_port());
	}
}

} // namespace DevicesManager

} // namespace Haruhi

