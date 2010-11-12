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
#include "controller_with_port_item.h"
#include "backend.h"


namespace Haruhi {

namespace EventBackend {

ControllerWithPortItem::ControllerWithPortItem (DeviceWithPortItem* parent, QString const& name):
	ControllerItem (parent, name),
	PortItem (parent->backend()),
	_device_item (parent),
	_learning (false)
{
	// Allocate new port:
	backend()->graph()->lock();
	_port = new EventPort (backend(), name.ascii(), Port::Output, parent->port_group());
	_device_item->controllers()->insert (this);
	backend()->graph()->unlock();
	// Fully constructed:
	set_ready (true);
}


ControllerWithPortItem::~ControllerWithPortItem()
{
	backend()->graph()->lock();
	_device_item->controllers()->erase (this);
	delete _port;
	backend()->graph()->unlock();
}


void
ControllerWithPortItem::learn()
{
	_learning = !_learning;
	if (_learning)
		setIcon (0, Config::Icons16::colorpicker());
	else
		setIcon (0, Config::Icons16::event_output_port());
}


void
ControllerWithPortItem::stop_learning()
{
	_learning = false;
	QApplication::postEvent (treeWidget(), new PortsListView::LearnedParams (this));
}


void
ControllerWithPortItem::update_name()
{
	_port->set_name (text (0).ascii());
}


QString
ControllerWithPortItem::name() const
{
	return text (0);
}


EventPort*
ControllerWithPortItem::port() const
{
	return _port;
}


bool
ControllerWithPortItem::handle_event (Transport::MidiEvent const& event)
{
	typedef Transport::MidiEvent MidiEvent;

	bool handled = false;
	Timestamp t = event.timestamp;
	EventBuffer* buffer = _port->event_buffer();
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
				buffer->push (new VoiceEvent (t, event.note_on.note, VoiceAuto,
											  (event.note_on.velocity == 0)? VoiceEvent::Release : VoiceEvent::Create,
											  VoiceEvent::frequency_from_key_id (event.note_on.note, backend()->graph()->master_tune()),
											  event.note_on.velocity / 127.0));
				buffer->push (new VoiceControllerEvent (t, event.note_on.note, event.note_on.velocity / 127.0));
				handled = true;
			}
			break;

		case MidiEvent::NoteOff:
			if (_note_filter && (_note_channel == 0 || _note_channel == event.note_off.channel + 1))
			{
				buffer->push (new VoiceControllerEvent (t, event.note_off.note, event.note_off.velocity / 127.0));
				buffer->push (new VoiceEvent (t, event.note_off.note, VoiceAuto, VoiceEvent::Release,
											  VoiceEvent::frequency_from_key_id (event.note_off.note, backend()->graph()->master_tune()),
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
					buffer->push (new ControllerEvent (t, value / 127.0));
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
				buffer->push (new ControllerEvent (t, event.pitchbend.value == 0 ? 0.5 : (event.pitchbend.value + 8192) / 16382.0));
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
					buffer->push (new ControllerEvent (t, value / 127.0));
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
					buffer->push (new VoiceControllerEvent (t, event.key_pressure.note, value / 127.0));
					handled = true;
				}
			}
			break;
	}

	return handled;
}


void
ControllerWithPortItem::load_state (QDomElement const& element)
{
	ControllerItem::load_state (element);
	update_name();
}

} // namespace EventBackend

} // namespace Haruhi

