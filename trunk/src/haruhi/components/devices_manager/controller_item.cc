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
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config.h>

// Local:
#include "controller_item.h"


namespace Haruhi {

namespace DevicesManagerPrivate {

ControllerItem::ControllerItem (DeviceItem* parent, QString const& name):
	Item (static_cast<QTreeWidgetItem*> (parent), name),
	_note_filter (false),
	_note_channel (0),
	_controller_filter (false),
	_controller_channel (0),
	_controller_number (0),
	_controller_invert (false),
	_pitchbend_filter (false),
	_pitchbend_channel (0),
	_channel_pressure_filter (false),
	_channel_pressure_channel (0),
	_channel_pressure_invert (false),
	_key_pressure_filter (false),
	_key_pressure_channel (0),
	_key_pressure_invert (false)
{
	// Configure item:
	setIcon (0, Config::Icons16::event_output_port());
	update_minimum_size();
}


ControllerItem::~ControllerItem()
{
	// Remove itself from list view:
	if (parent())
		parent()->takeChild (parent()->indexOfChild (this));
}


void
ControllerItem::save_state (QDomElement& element) const
{
	element.setAttribute ("name", name());

	QDomElement note_filter = element.ownerDocument().createElement ("note-filter");
	note_filter.setAttribute ("enabled", _note_filter ? "true" : "false");
	note_filter.setAttribute ("channel", _note_channel == 0 ? "all" : QString ("%1").arg (_note_channel));

	QDomElement controller_filter = element.ownerDocument().createElement ("controller-filter");
	controller_filter.setAttribute ("enabled", _controller_filter ? "true" : "false");
	controller_filter.setAttribute ("channel", _controller_filter == 0 ? "all" : QString ("%1").arg (_controller_channel));
	controller_filter.setAttribute ("controller-number", QString ("%1").arg (_controller_number));
	controller_filter.setAttribute ("controller-invert", _controller_invert ? "true" : "false");

	QDomElement pitchbend_filter = element.ownerDocument().createElement ("pitchbend-filter");
	pitchbend_filter.setAttribute ("enabled", _pitchbend_filter ? "true" : "false");
	pitchbend_filter.setAttribute ("channel", _pitchbend_channel == 0 ? "all" : QString ("%1").arg (_pitchbend_channel));

	QDomElement channel_pressure_filter = element.ownerDocument().createElement ("channel-pressure");
	channel_pressure_filter.setAttribute ("enabled", _channel_pressure_filter ? "true" : "false");
	channel_pressure_filter.setAttribute ("channel", _channel_pressure_channel == 0 ? "all" : QString ("%1").arg (_channel_pressure_channel));
	channel_pressure_filter.setAttribute ("invert", _channel_pressure_invert ? "true" : "false");

	element.appendChild (note_filter);
	element.appendChild (controller_filter);
	element.appendChild (pitchbend_filter);
	element.appendChild (channel_pressure_filter);
}


void
ControllerItem::load_state (QDomElement const& element)
{
	setText (0, element.attribute ("name"));

	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "note-filter")
			{
				_note_filter = e.attribute ("enabled") == "true";
				_note_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
			}
			else if (e.tagName() == "controller-filter")
			{
				_controller_filter = e.attribute ("enabled") == "true";
				_controller_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
				_controller_number = e.attribute ("controller-number").toInt();
				_controller_invert = e.attribute ("controller-invert") == "true";
			}
			else if (e.tagName() == "pitchbend-filter")
			{
				_pitchbend_filter = e.attribute ("enabled") == "true";
				_pitchbend_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
			}
			else if (e.tagName() == "channel-pressure")
			{
				_channel_pressure_filter = e.attribute ("enabled") == "true";
				_channel_pressure_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
				_channel_pressure_invert = e.attribute ("invert") == "true";
			}
		}
	}
}

} // namespace DevicesManagerPrivate

} // namespace Haruhi

