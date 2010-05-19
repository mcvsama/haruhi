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
#include <QtGui/QMessageBox>
#include <QtGui/QHeaderView>
#include <QtGui/QTreeWidgetItem>

// Local:
#include "port_item.h"
#include "event_backend.h"


namespace Haruhi {

namespace EventBackendPrivate {

PortItem::PortItem (PortsListView* parent, QString const& name):
	QTreeWidgetItem (parent, QStringList (name)),
	_backend (parent->backend()),
	_port_item (0),
	_ready (false)
{
	update_minimum_size();
}


PortItem::PortItem (PortItem* parent, QString const& name):
	QTreeWidgetItem (parent, QStringList (name)),
	_backend (parent->_backend),
	_port_item (parent),
	_ready (false)
{
	update_minimum_size();
}


PortItem::~PortItem()
{
	// Remove itself from list view:
	if (parent())
		parent()->takeChild (parent()->indexOfChild (this));
}


void
PortItem::update_minimum_size()
{
	QSize s = sizeHint (0);
	if (s.height() < 18)
	{
		s.setHeight (18);
		setSizeHint (0, s);
	}
}

} // namespace EventBackendPrivate

} // namespace Haruhi

