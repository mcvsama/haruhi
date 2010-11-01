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
#include "device_with_port_item.h"


namespace Haruhi {

namespace EventBackendPrivate {

PortsListView::PortsListView (QWidget* parent, EventBackend* backend):
	DevicesManager::PortsListView (parent),
	_backend (backend)
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
	setHeaderLabel ("Session devices");
}


DeviceItem*
PortsListView::create_device_item (QString const& name)
{
	return new DeviceWithPortItem (_backend, this, name);
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
		lp->item->setIcon (0, Config::Icons16::event_output_port());
	}
}

} // namespace EventBackendPrivate

} // namespace Haruhi

