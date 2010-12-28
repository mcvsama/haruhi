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
#include <haruhi/config/all.h>

// Local:
#include "tree.h"
#include "input_item.h"
#include "output_item.h"


namespace Haruhi {

namespace AudioBackendImpl {

Tree::Tree (QWidget* parent, Backend* backend, const char* header_title):
	QTreeWidget (parent),
	_backend (backend)
{
	header()->setClickable (false);
	header()->setResizeMode (QHeaderView::Stretch);
	setMinimumWidth (160);
	setSortingEnabled (true);
	sortByColumn (0, Qt::AscendingOrder);
	setSelectionMode (QTreeWidget::SingleSelection);
	setRootIsDecorated (false);
	setAllColumnsShowFocus (true);
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
	setContextMenuPolicy (Qt::CustomContextMenu);
	setHeaderLabel (header_title);
}


QTreeWidgetItem*
Tree::selected_item() const
{
	QList<QTreeWidgetItem*> list = selectedItems();
	return list.empty() ? 0 : list.front();
}


void
Tree::save_state (QDomElement& element) const
{
	for (int i = 0; i < invisibleRootItem()->childCount(); ++i)
	{
		QTreeWidgetItem* item = invisibleRootItem()->child (i);
		InputItem* input_item = dynamic_cast<InputItem*> (item);
		OutputItem* output_item = dynamic_cast<OutputItem*> (item);
		PortItem* port_item = 0;
		QDomElement e;

		if (input_item)
		{
			port_item = input_item;
			e = element.ownerDocument().createElement ("input");
		}
		else if (output_item)
		{
			port_item = output_item;
			e = element.ownerDocument().createElement ("output");
		}

		if (port_item)
		{
			port_item->save_state (e);
			element.appendChild (e);
		}
	}
}


void
Tree::load_state (QDomElement const& element)
{
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "input")
			{
				InputItem* port = new InputItem (this, e.attribute ("name"));
				port->load_state (e);
			}
			else if (e.tagName() == "output")
			{
				OutputItem* port = new OutputItem (this, e.attribute ("name"));
				port->load_state (e);
			}
		}
	}
}

} // namespace AudioBackendImpl

} // namespace Haruhi

