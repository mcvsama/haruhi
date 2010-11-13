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

#ifndef HARUHI__COMPONENTS__PORTS_CONNECTOR__PORT_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__PORTS_CONNECTOR__PORT_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/graph/port.h>
#include <haruhi/graph/port_group.h>

// Local:
#include "comparable_item.h"
#include "highlightable_item.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

class PortItem:
	public ComparableItem,
	public HighlightableItem
{
	friend class ComparableItem;

  public:
	PortItem (Port::Direction type, Port* port, QTreeWidgetItem* parent, QString const& label);

	~PortItem();

	Port*
	port() const;

	void
	update();

  protected:
	Port::Direction	_type;
	Port*			_port;
};

} // namespace PortsConnectorPrivate

} // namespace Haruhi

#endif

