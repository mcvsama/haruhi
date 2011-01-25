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
#include <QtGui/QPixmap>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_port.h>
#include <haruhi/graph/event_port.h>

// Local:
#include "port_item.h"
#include "group_item.h"
#include "ports_list.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

PortItem::PortItem (Port::Direction type, Port* port, QTreeWidgetItem* parent, QString const& label):
	QTreeWidgetItem (parent, QStringList (label)),
	_type (type),
	_port (port)
{
	QSize s = sizeHint (0);
	if (s.height() < 18)
	{
		s.setHeight (18);
		setSizeHint (0, s);
	}

	if (dynamic_cast<AudioPort*> (port))
	{
		switch (_type)
		{
			case Port::Input:	setIcon (0, Resources::Icons16::audio_input_port()); break;
			case Port::Output:	setIcon (0, Resources::Icons16::audio_output_port()); break;
		}
	}
	else
	{
		if (port->has_flags (Port::Polyphonic))
		{
			switch (_type)
			{
				case Port::Input:	setIcon (0, Resources::Icons16::event_polyphonic_input_port()); break;
				case Port::Output:	setIcon (0, Resources::Icons16::event_polyphonic_output_port()); break;
			}
		}
		else
		{
			switch (_type)
			{
				case Port::Input:	setIcon (0, Resources::Icons16::event_input_port()); break;
				case Port::Output:	setIcon (0, Resources::Icons16::event_output_port()); break;
			}
		}
	}
}


PortItem::~PortItem()
{
	// Call forget_item on ports_connector() so it will remove PortItem
	// from cache and prevent crashing while trying to unhighlight connected
	// but nonexistent port.
	QTreeWidget* tree = treeWidget();
	if (tree)
	{
		PortsList* ports_list = dynamic_cast<PortsList*> (tree);
		if (ports_list)
		{
			ports_list->ports_connector()->forget_item (this);
		}
	}
}


Port*
PortItem::port() const
{
	return _port;
}


void
PortItem::update()
{
	setText (0, QString::fromStdString (_port->name()));
	// Update parent if it is GroupItem: FIXME causes multiple updates on GroupItem if there is many ports in that group:
	GroupItem* g = dynamic_cast<GroupItem*> (parent());
	if (g)
		g->update();
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

