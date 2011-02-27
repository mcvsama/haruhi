/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
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
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/session/unit_bay.h>

// Local:
#include "unit_item.h"
#include "ports_list.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

UnitItem::UnitItem (Port::Direction type, Unit* unit, QTreeWidget* parent, QString const& label):
	QTreeWidgetItem (parent, QStringList (label)),
	_filtered_out (false),
	_type (type),
	_unit (unit)
{
	QSize s = sizeHint (0);
	if (s.height() < 18)
	{
		s.setHeight (18);
		setSizeHint (0, s);
	}

	if (dynamic_cast<UnitBay const*> (_unit))
	{
		switch (type)
		{
			case Port::Input:	setIcon (0, Resources::Icons16::input_unit_bay()); break;
			case Port::Output:	setIcon (0, Resources::Icons16::output_unit_bay()); break;
		}
	}
	else
	{
		switch (type)
		{
			case Port::Input:	setIcon (0, Resources::Icons16::input_unit()); break;
			case Port::Output:	setIcon (0, Resources::Icons16::output_unit()); break;
		}
	}

	update();
	update_visibility();
}


Unit*
UnitItem::unit() const
{
	return _unit;
}


void
UnitItem::update()
{
	QString text;
	if (_unit->id() >= Unit::ReservedID)
		text = QString::fromStdString (_unit->title());
	else
		text = QString ("%1: %2").arg (_unit->id()).arg (QString::fromStdString (_unit->title()));
	setText (0, text);
}


PortItem*
UnitItem::insert_port (Port* port)
{
	if (!port_exist (port))
	{
		QTreeWidgetItem* parent = port->group() ? static_cast<QTreeWidgetItem*> (find_or_create_group_item_for (port->group())) : static_cast<QTreeWidgetItem*> (this);
		PortItem* item = new PortItem (_type, port, parent, QString::fromStdString (port->name()));
		parent->addChild (item);
		_ports[port] = item;
		update_visibility();
		return item;
	}
	return 0;
}


void
UnitItem::remove_port (Port* port)
{
	if (port_exist (port))
	{
		PortsToItemsMap::iterator k = _ports.find (port);
		if (k != _ports.end())
		{
			k->second->parent()->takeChild (k->second->indexOfChild (k->second));
			delete (k->second);
			_ports.erase (k);
			if (port->group())
				cleanup_group (port->group());
		}
		update_visibility();
	}
}


void
UnitItem::update_port (Port* port)
{
	_ports[port]->update();
}


bool
UnitItem::port_exist (Port* port) const
{
	return _ports.find (port) != _ports.end();
}


void
UnitItem::set_filtered_out (bool set)
{
	_filtered_out = set;
	update_visibility();
}


GroupItem*
UnitItem::find_or_create_group_item_for (PortGroup* group)
{
	GroupItem* item = 0;
	for (int i = 0; i < childCount(); ++i)
	{
		QTreeWidgetItem* c = child (i);
		if ((item = dynamic_cast<GroupItem*> (c)) != 0 && item->group() == group)
			return item;
	}
	// If group item not found, create new:
	item = new GroupItem (group, this);
	addChild (item);
	return item;
}


void
UnitItem::cleanup_group (PortGroup* group)
{
	GroupItem* item = find_or_create_group_item_for (group);
	if (item->childCount() == 0)
	{
		item->parent()->takeChild (item->parent()->indexOfChild (item));
		delete item;
	}
}


void
UnitItem::update_visibility()
{
	setHidden (childCount() == 0 || _filtered_out);
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

