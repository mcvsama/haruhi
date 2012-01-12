/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <algorithm>
#include <iterator>
#include <set>

// Qt:
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/session/unit_bay.h>

// Local:
#include "unit_item.h"
#include "ports_list.h"
#include "ports_connector.h"


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
	read_ports();
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


void
UnitItem::read_ports()
{
	Ports u_ports = (_type == Port::Input) ? _unit->inputs() : _unit->outputs(); // Unit ports
	Ports l_ports; // UnitItem ports
	for (auto p: _ports)
		l_ports.insert (p.first);

	Ports added;
	Ports removed;
	Ports rest;
	std::set_difference (u_ports.begin(), u_ports.end(), l_ports.begin(), l_ports.end(), std::inserter (added, added.end()));
	std::set_difference (l_ports.begin(), l_ports.end(), u_ports.begin(), u_ports.end(), std::inserter (removed, removed.end()));
	std::set_intersection (u_ports.begin(), u_ports.end(), l_ports.begin(), l_ports.end(), std::inserter (rest, rest.end()));

	// Since some operations (setHidden) may call sort() and thus operator< which operatoes on Port,
	// remove deleted items first to avoid segmentation faults.
	for (Port* p: removed)
		remove_port (p);
	for (Port* p: added)
		insert_port (p);
	for (Port* p: rest)
		update_port (p);
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
		// Highlighting helper:
		ports_connector()->_ports_to_items[port] = item;
		return item;
	}
	return 0;
}


void
UnitItem::remove_port (Port* port)
{
	if (port_exist (port))
	{
		// Highlighting helper:
		ports_connector()->_ports_to_items.erase (port);

		PortsToItemsMap::iterator k = _ports.find (port);
		if (k != _ports.end())
		{
			PortItem* port_item = k->second;
			GroupItem* group_item = 0;
			if (port_item->parent())
				group_item = dynamic_cast<GroupItem*> (port_item->parent());

			port_item->parent()->removeChild (k->second);
			delete (port_item);
			_ports.erase (k);

			if (group_item)
				cleanup_group_item (group_item);
		}
		update_visibility();
	}
}


void
UnitItem::update_port (Port* port)
{
	PortItem* port_item = _ports[port];
	GroupItem* group_item = dynamic_cast<GroupItem*> (port_item->parent());
	bool changed = false;

	// If unit changed or gained its group, take it out of current parent:
	if (group_item && group_item->group() != port->group())
	{
		group_item->removeChild (port_item);
		cleanup_group_item (group_item);
		group_item = 0;
		changed = true;
	}
	else if (!group_item && port->group())
	{
		port_item->parent()->removeChild (port_item);
		changed = true;
	}

	// Lost its group?
	if (changed)
	{
		if (port->group() == 0)
			addChild (port_item);
		// Moved to new group?
		else
		{
			if (port_item->parent())
				port_item->parent()->removeChild (port_item);
			group_item = find_or_create_group_item_for (port->group());
			group_item->addChild (port_item);
		}
	}

	port_item->update();
	// Update group title if changed:
	if (group_item)
		group_item->update();
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
UnitItem::find_group_item_for (PortGroup* group)
{
	GroupItem* item = 0;
	for (int i = 0; i < childCount(); ++i)
	{
		QTreeWidgetItem* c = child (i);
		if ((item = dynamic_cast<GroupItem*> (c)) != 0 && item->group() == group)
			return item;
	}
	return 0;
}


GroupItem*
UnitItem::find_or_create_group_item_for (PortGroup* group)
{
	GroupItem* item = find_group_item_for (group);
	if (item)
		return item;
	// If group item not found, create new:
	item = new GroupItem (group, this);
	addChild (item);
	return item;
}


void
UnitItem::cleanup_group (PortGroup* group)
{
	GroupItem* item = find_group_item_for (group);
	if (item)
	   cleanup_group_item (item);
}


void
UnitItem::cleanup_group_item (GroupItem* group_item)
{
	if (group_item->childCount() == 0)
	{
		group_item->parent()->removeChild (group_item);
		delete group_item;
	}
}


void
UnitItem::update_visibility()
{
	setHidden (childCount() == 0 || _filtered_out);
}


PortsConnector*
UnitItem::ports_connector() const
{
	PortsList* pl = dynamic_cast<PortsList*> (treeWidget());
	if (pl)
		return pl->ports_connector();
	return 0;
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

