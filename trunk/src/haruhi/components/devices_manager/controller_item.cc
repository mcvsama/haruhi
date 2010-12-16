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
#include <QtGui/QApplication>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/lib/midi.h>

// Local:
#include "controller_item.h"


namespace Haruhi {

namespace DevicesManager {

ControllerItem::ControllerItem (DeviceItem* parent, QString const& name):
	Item (static_cast<QTreeWidgetItem*> (parent), name),
	note_filter (false),
	note_channel (0),
	controller_filter (false),
	controller_channel (0),
	controller_number (0),
	controller_invert (false),
	pitchbend_filter (false),
	pitchbend_channel (0),
	channel_pressure_filter (false),
	channel_pressure_channel (0),
	channel_pressure_invert (false),
	key_pressure_filter (false),
	key_pressure_channel (0),
	key_pressure_invert (false),
	_learning (false)
{
	// Configure item:
	setIcon (0, Resources::Icons16::event_output_port());
	update_minimum_size();
}


ControllerItem::~ControllerItem()
{
	// Remove itself from list view:
	if (parent())
		parent()->takeChild (parent()->indexOfChild (this));
}


void
ControllerItem::learn()
{
	_learning = !_learning;
	if (_learning)
		setIcon (0, Resources::Icons16::colorpicker());
	else
		setIcon (0, Resources::Icons16::event_output_port());
}


void
ControllerItem::stop_learning()
{
	_learning = false;
	// Called from JACK thread, we should not change icon directly:
	QApplication::postEvent (treeWidget(), new PortsListView::LearnedParams (this));
}


void
ControllerItem::learn_from_event (MIDI::Event const& event)
{
	switch (event.type)
	{
		case MIDI::Event::NoteOn:
		case MIDI::Event::NoteOff:
			note_filter = true;
			note_channel = (event.type == MIDI::Event::NoteOn ? event.note_on.channel : event.note_off.channel) + 1;
			controller_filter = false;
			pitchbend_filter = false;
			channel_pressure_filter = false;
			stop_learning();
			break;

		case MIDI::Event::Controller:
			note_filter = false;
			controller_filter = true;
			controller_channel = event.controller.channel + 1;
			controller_number = event.controller.number;
			controller_invert = false;
			pitchbend_filter = false;
			channel_pressure_filter = false;
			stop_learning();
			break;

		case MIDI::Event::Pitchbend:
			note_filter = false;
			controller_filter = false;
			pitchbend_filter = true;
			pitchbend_channel = event.pitchbend.channel + 1;
			channel_pressure_filter = false;
			stop_learning();
			break;

		case MIDI::Event::ChannelPressure:
			note_filter = false;
			controller_filter = false;
			pitchbend_filter = false;
			channel_pressure_filter = true;
			channel_pressure_channel = event.channel_pressure.channel + 1;
			stop_learning();
			break;

		default:
			break;
	}
}


void
ControllerItem::save_state (QDomElement& element) const
{
	element.setAttribute ("name", name());

	QDomElement note_filter_el = element.ownerDocument().createElement ("note-filter");
	note_filter_el.setAttribute ("enabled", note_filter ? "true" : "false");
	note_filter_el.setAttribute ("channel", note_channel == 0 ? "all" : QString ("%1").arg (note_channel));

	QDomElement controller_filter_el = element.ownerDocument().createElement ("controller-filter");
	controller_filter_el.setAttribute ("enabled", controller_filter ? "true" : "false");
	controller_filter_el.setAttribute ("channel", controller_filter == 0 ? "all" : QString ("%1").arg (controller_channel));
	controller_filter_el.setAttribute ("controller-number", QString ("%1").arg (controller_number));
	controller_filter_el.setAttribute ("controller-invert", controller_invert ? "true" : "false");

	QDomElement pitchbend_filter_el = element.ownerDocument().createElement ("pitchbend-filter");
	pitchbend_filter_el.setAttribute ("enabled", pitchbend_filter ? "true" : "false");
	pitchbend_filter_el.setAttribute ("channel", pitchbend_channel == 0 ? "all" : QString ("%1").arg (pitchbend_channel));

	QDomElement channel_pressure_filter_el = element.ownerDocument().createElement ("channel-pressure");
	channel_pressure_filter_el.setAttribute ("enabled", channel_pressure_filter ? "true" : "false");
	channel_pressure_filter_el.setAttribute ("channel", channel_pressure_channel == 0 ? "all" : QString ("%1").arg (channel_pressure_channel));
	channel_pressure_filter_el.setAttribute ("invert", channel_pressure_invert ? "true" : "false");

	element.appendChild (note_filter_el);
	element.appendChild (controller_filter_el);
	element.appendChild (pitchbend_filter_el);
	element.appendChild (channel_pressure_filter_el);
}


void
ControllerItem::load_state (QDomElement const& element)
{
	setText (0, element.attribute ("name"));

	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (e.isNull())
			continue;

		if (e.tagName() == "note-filter")
		{
			note_filter = e.attribute ("enabled") == "true";
			note_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
		}
		else if (e.tagName() == "controller-filter")
		{
			controller_filter = e.attribute ("enabled") == "true";
			controller_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
			controller_number = e.attribute ("controller-number").toInt();
			controller_invert = e.attribute ("controller-invert") == "true";
		}
		else if (e.tagName() == "pitchbend-filter")
		{
			pitchbend_filter = e.attribute ("enabled") == "true";
			pitchbend_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
		}
		else if (e.tagName() == "channel-pressure")
		{
			channel_pressure_filter = e.attribute ("enabled") == "true";
			channel_pressure_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
			channel_pressure_invert = e.attribute ("invert") == "true";
		}
	}
}

} // namespace DevicesManager

} // namespace Haruhi

