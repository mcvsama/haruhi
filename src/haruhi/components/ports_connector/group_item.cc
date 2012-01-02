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

// Qt:
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "group_item.h"
#include "unit_item.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

GroupItem::GroupItem (PortGroup* group, UnitItem* parent):
	QTreeWidgetItem (parent, QStringList (QString::fromStdString (group->name()))),
	_group (group)
{
	QSize s = sizeHint (0);
	if (s.height() < 18)
	{
		s.setHeight (18);
		setSizeHint (0, s);
	}

	setIcon (0, Resources::Icons16::port_group());
}


PortGroup*
GroupItem::group() const
{
	return _group;
}


void
GroupItem::update()
{
	setText (0, QString::fromStdString (_group->name()));
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

