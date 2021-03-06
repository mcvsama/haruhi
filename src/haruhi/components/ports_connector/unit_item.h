/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__COMPONENTS__PORTS_CONNECTOR__UNIT_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__PORTS_CONNECTOR__UNIT_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/port.h>
#include <haruhi/graph/unit.h>

// Local:
#include "port_item.h"
#include "group_item.h"
#include "highlightable_item.h"


namespace Haruhi {

class PortsConnector;

namespace PortsConnectorPrivate {

class UnitItem:
	virtual public QTreeWidgetItem,
	public HighlightableItem
{
  public:
	typedef std::map<Port*, PortItem*>  PortsToItemsMap;

	enum Graph { Audio, Event };

  public:
	UnitItem (Port::Direction, Unit*, QTreeWidget* parent, QString const& label);

	Unit*
	unit() const;

	void
	update();

	void
	read_ports();

	PortItem*
	insert_port (Port*);

	void
	remove_port (Port*);

	void
	update_port (Port*);

	bool
	port_exist (Port*) const;

	void
	set_filtered_out (bool set);

	template<class Predicate>
		int
		count_outputs_if (Predicate predicate) const;

	template<class Predicate>
		int
		count_inputs_if (Predicate predicate) const;

  private:
	/**
	 * Returns group item for given group or 0.
	 */
	GroupItem*
	find_group_item_for (PortGroup*);

	/**
	 * Returns group item for given group or creates
	 * new if one doesn't exist.
	 */
	GroupItem*
	find_or_create_group_item_for (PortGroup*);

	/**
	 * Removes group item for given group if it is empty.
	 */
	void
	cleanup_group (PortGroup*);

	/**
	 * Removes group item if it is empty.
	 */
	void
	cleanup_group_item (GroupItem*);

	/**
	 * Hides unit item if it has no children ports.
	 */
	void
	update_visibility();

	PortsConnector*
	ports_connector() const;

  public:
	PortsToItemsMap	_ports;

  private:
	bool			_filtered_out;
	Port::Direction	_type;
	Unit*			_unit;
};


template<class Predicate>
	inline int
	UnitItem::count_outputs_if (Predicate predicate) const
	{
		return std::count_if (unit()->outputs().begin(), unit()->outputs().end(), predicate);
	}


template<class Predicate>
	inline int
	UnitItem::count_inputs_if (Predicate predicate) const
	{
		return std::count_if (unit()->inputs().begin(), unit()->inputs().end(), predicate);
	}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

#endif

