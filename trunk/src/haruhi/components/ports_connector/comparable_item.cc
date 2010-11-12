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
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/core/audio_port.h>
#include <haruhi/core/event_port.h>

// Local:
#include "comparable_item.h"
#include "port_item.h"
#include "group_item.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

ComparableItem::ComparableItem():
	QTreeWidgetItem (static_cast<QTreeWidget*> (0))
{
}


bool
ComparableItem::operator< (QTreeWidgetItem const& other) const
{
	QTreeWidgetItem const* sources[2] = { this, &other };
	int numbers[2] = { 0, 0 };
	PortItem const* port_item;

	for (int i = 0; i < 2; ++i)
	{
		if (dynamic_cast<GroupItem const*> (sources[i]))
			numbers[i] = 0;
		else if ((port_item = dynamic_cast<PortItem const*> (sources[i])))
		{
			if (dynamic_cast<EventPort const*> (port_item->_port))
				numbers[i] = 1;
			else if (dynamic_cast<AudioPort const*> (port_item->_port))
				numbers[i] = 2;
			else
				numbers[i] = 3;
		}
		else
			numbers[i] = 3;
	}

	return (numbers[0] < numbers[1]) || (numbers[0] == numbers[1] && QTreeWidgetItem::operator< (other));
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

