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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_port.h>
#include <haruhi/graph/event_port.h>

// Local:
#include "comparable_item.h"
#include "port_item.h"
#include "group_item.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

HighlightableItem::HighlightableItem():
	QTreeWidgetItem (static_cast<QTreeWidget*> (0)),
	_highlighted (false),
	_highlighted_subitems (0)
{
	_normal_foreground = foreground (0);
	_normal_background = background (0);
	_highlighted_foreground = QBrush (QColor (0x00, 0x00, 0x00));
	_highlighted_background = QBrush (QColor (0xec, 0xf4, 0xff));
}


void
HighlightableItem::set_highlighted (bool set)
{
	if (_highlighted != set)
	{
		if (parent())
		{
			HighlightableItem* ci = dynamic_cast<HighlightableItem*> (parent());
			if (ci)
			{
				ci->_highlighted_subitems += set ? +1 : -1;
				ci->set_highlighted (ci->_highlighted_subitems > 0);
			}
		}
		_highlighted = set;
		update_highlight();
	}
}


void
HighlightableItem::update_highlight()
{
	setForeground (0, (_highlighted && (!isExpanded() || childCount() == 0)) ? _highlighted_foreground : _normal_foreground);
	setBackground (0, (_highlighted && (!isExpanded() || childCount() == 0)) ? _highlighted_background : _normal_background);
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

