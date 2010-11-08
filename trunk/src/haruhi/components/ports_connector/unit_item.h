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

#ifndef HARUHI__COMPONENTS__PORTS_CONNECTOR__UNIT_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__PORTS_CONNECTOR__UNIT_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/core/port.h>
#include <haruhi/core/unit.h>

// Local:
#include "port_item.h"
#include "group_item.h"
#include "highlightable_item.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

class UnitItem:
	virtual public QTreeWidgetItem,
	public HighlightableItem
{
  public:
	typedef std::map<Core::Port*, PortItem*>  PortsToItemsMap;

	enum Graph { Audio, Event };

  public:
	UnitItem (Core::Port::Direction, Core::Unit*, QTreeWidget* parent, QString const& label);

	Core::Unit*
	unit() const;

	void
	update();

	PortItem*
	insert_port (Core::Port*);

	void
	remove_port (Core::Port*);

	void
	update_port (Core::Port*);

	bool
	port_exist (Core::Port*) const;

	template<class Predicate>
		inline int
		count_outputs_if (Predicate predicate) const
		{
			return std::count_if (unit()->outputs().begin(), unit()->outputs().end(), predicate);
		}

	template<class Predicate>
		inline int
		count_inputs_if (Predicate predicate) const
		{
			return std::count_if (unit()->inputs().begin(), unit()->inputs().end(), predicate);
		}

  private:
	/**
	 * Returns group item for given group or creates
	 * new if one doesn't exist.
	 */
	GroupItem*
	find_or_create_group_item_for (Core::PortGroup*);

	/**
	 * Removes group item if it is empty.
	 */
	void
	cleanup_group (Core::PortGroup*);

  public:
	PortsToItemsMap			_ports;

  private:
	Core::Port::Direction	_type;
	Core::Unit*				_unit;
};

} // namespace PortsConnectorPrivate

} // namespace Haruhi

#endif

