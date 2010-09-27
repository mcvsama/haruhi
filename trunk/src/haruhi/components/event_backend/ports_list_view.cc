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
#include "external_input_item.h"


namespace Haruhi {

namespace EventBackendPrivate {

PortsListView::PortsListView (QWidget* parent, EventBackend* backend, const char* header_title):
	QTreeWidget (parent),
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
	setHeaderLabel (header_title);
}


QTreeWidgetItem*
PortsListView::selected_item() const
{
	QList<QTreeWidgetItem*> list = selectedItems();
	return list.empty() ? 0 : list.front();
}


void
PortsListView::save_state (QDomElement& element) const
{
	for (int i = 0; i < invisibleRootItem()->childCount(); ++i)
	{
		ExternalInputItem* external_input_item = dynamic_cast<ExternalInputItem*> (invisibleRootItem()->child (i));
		PortItem* port_item = 0;
		QDomElement e;

		if (external_input_item)
		{
			port_item = external_input_item;
			e = element.ownerDocument().createElement ("external-input");
		}

		if (port_item)
		{
			port_item->save_state (e);
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
			if (e.tagName() == "external-input")
			{
				ExternalInputItem* port = new ExternalInputItem (this, e.attribute ("name"));
				port->load_state (e);
			}
		}
	}
}


void
PortsListView::customEvent (QEvent* event)
{
	LearnedParams* lp = dynamic_cast<LearnedParams*> (event);
	if (lp)
	{
		// Force update of editor dialog:
		lp->port_item->setSelected (false);
		lp->port_item->setSelected (true);
		// Reset icon:
		lp->port_item->setIcon (0, Config::Icons16::event_output_port());
	}
}

} // namespace EventBackendPrivate

} // namespace Haruhi

