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
#include "internal_input_item.h"
#include "event_backend.h"


namespace Haruhi {

namespace EventBackendPrivate {

InternalInputItem::InternalInputItem (ExternalInputItem* parent, QString const& name):
	PortItem (parent, name),
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
	_key_pressure_invert (false),
	_learning (false)
{
	// Allocate new port:
	_backend->graph()->lock();
	_port = new Core::EventPort (_backend, name.ascii(), Core::Port::Output, parent->port_group());
	if (port_item())
		port_item()->internal_inputs()->insert (this);
	_backend->graph()->unlock();
	// Configure item:
	setIcon (0, Config::Icons16::event_output_port());
	// Fully constructed:
	set_ready (true);
}


InternalInputItem::~InternalInputItem()
{
	_backend->graph()->lock();
	if (port_item())
		port_item()->internal_inputs()->erase (this);
	delete _port;
	_backend->graph()->unlock();
}


void
InternalInputItem::learn()
{
	_learning = !_learning;
	if (_learning)
		setIcon (0, Config::Icons16::colorpicker());
	else
		setIcon (0, Config::Icons16::event_output_port());
}


void
InternalInputItem::stop_learning()
{
	_learning = false;
	QApplication::postEvent (treeWidget(), new PortsListView::LearnedParams (this));
}


void
InternalInputItem::update_name()
{
	_port->set_name (text (0).ascii());
}


QString
InternalInputItem::name() const
{
	return text (0);
}


Core::EventPort*
InternalInputItem::port() const
{
	return _port;
}


bool
InternalInputItem::handle_event (EventTransport::MidiEvent const& event)
{
	typedef EventTransport::MidiEvent MidiEvent;

	bool handled = false;
	Core::Timestamp t = event.timestamp;
	Core::EventBuffer* buffer = _port->event_buffer();
	switch (event.type)
	{
		case MidiEvent::NoteOn:
			if (_learning)
			{
				_note_filter = true;
				_note_channel = event.note_on.channel + 1;
				_controller_filter = false;
				_pitchbend_filter = false;
				_channel_pressure_filter = false;
				stop_learning();
			}
			if (_note_filter && (_note_channel == 0 || _note_channel == event.note_on.channel + 1))
			{
				buffer->push (new Core::VoiceEvent (t, event.note_on.note, Core::VoiceAuto,
													(event.note_on.velocity == 0)? Core::VoiceEvent::Release : Core::VoiceEvent::Create,
													Core::VoiceEvent::frequency_from_key_id (event.note_on.note, _backend->graph()->master_tune()),
													event.note_on.velocity / 127.0));
				buffer->push (new Core::VoiceControllerEvent (t, event.note_on.note, event.note_on.velocity / 127.0));
				handled = true;
			}
			break;

		case MidiEvent::NoteOff:
			if (_note_filter && (_note_channel == 0 || _note_channel == event.note_off.channel + 1))
			{
				buffer->push (new Core::VoiceControllerEvent (t, event.note_off.note, event.note_off.velocity / 127.0));
				buffer->push (new Core::VoiceEvent (t, event.note_off.note, Core::VoiceAuto, Core::VoiceEvent::Release,
													Core::VoiceEvent::frequency_from_key_id (event.note_off.note, _backend->graph()->master_tune()),
													event.note_off.velocity / 127.0));
				handled = true;
			}
			break;

		case MidiEvent::Controller:
			if (_learning)
			{
				_note_filter = false;
				_controller_filter = true;
				_controller_channel = event.controller.channel + 1;
				_controller_number = event.controller.number;
				_controller_invert = false;
				_pitchbend_filter = false;
				_channel_pressure_filter = false;
				stop_learning();
			}
			// Block:
			{
				int value = event.controller.value;
				if (_controller_invert)
					value = 127 - value;
				if (_controller_filter && (_controller_channel == 0 || _controller_channel == event.controller.channel + 1) && _controller_number == static_cast<int> (event.controller.number))
				{
					buffer->push (new Core::ControllerEvent (t, value / 127.0));
					handled = true;
				}
			}
			break;

		case MidiEvent::Pitchbend:
			if (_learning)
			{
				_note_filter = false;
				_controller_filter = false;
				_pitchbend_filter = true;
				_pitchbend_channel = event.pitchbend.channel + 1;
				_channel_pressure_filter = false;
				stop_learning();
			}
			if (_pitchbend_filter && (_pitchbend_channel == 0 || _pitchbend_channel == event.pitchbend.channel + 1))
			{
				buffer->push (new Core::ControllerEvent (t, event.pitchbend.value == 0 ? 0.5 : (event.pitchbend.value + 8192) / 16382.0));
				handled = true;
			}
			break;

		case MidiEvent::ChannelPressure:
			if (_learning)
			{
				_note_filter = false;
				_controller_filter = false;
				_pitchbend_filter = false;
				_channel_pressure_filter = true;
				_channel_pressure_channel = event.channel_pressure.channel + 1;
				stop_learning();
			}
			// Block:
			{
				int value = event.channel_pressure.value;
				if (_channel_pressure_invert)
					value = 127 - value;
				if (_channel_pressure_filter && (_channel_pressure_channel == 0 || _channel_pressure_channel == event.channel_pressure.channel + 1))
				{
					buffer->push (new Core::ControllerEvent (t, value / 127.0));
					handled = true;
				}
			}
			break;

		case MidiEvent::KeyPressure:
			{
				int value = event.key_pressure.value;
				if (_key_pressure_invert)
					value = 127 - value;
				if (_key_pressure_filter && (_key_pressure_channel == 0 || _key_pressure_channel == event.key_pressure.channel + 1))
				{
					buffer->push (new Core::VoiceControllerEvent (t, event.key_pressure.note, value / 127.0));
					handled = true;
				}
			}
			break;
	}

	return handled;
}


void
InternalInputItem::save_state (QDomElement& element) const
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
InternalInputItem::load_state (QDomElement const& element)
{
	setText (0, element.attribute ("name"));
	update_name();

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

} // namespace EventBackendPrivate

} // namespace Haruhi

