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
#include <haruhi/config/all.h>
#include <haruhi/lib/midi.h>

// Local:
#include "controller_with_port_item.h"
#include "backend.h"


namespace Haruhi {

namespace EventBackendImpl {

ControllerWithPortItem::ControllerWithPortItem (DeviceWithPortItem* parent, QString const& name):
	ControllerItem (parent, name),
	PortItem (parent->backend()),
	_device_item (parent)
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
ControllerWithPortItem::handle_event (MIDI::Event const& event)
{
	if (learning())
		learn_from_event (event);

	bool handled = false;
	Timestamp t = event.timestamp;
	EventBuffer* buffer = _port->event_buffer();
	switch (event.type)
	{
		case MIDI::Event::NoteOn:
			if (note_filter && (note_channel == 0 || note_channel == event.note_on.channel + 1))
			{
				buffer->push (new VoiceEvent (t, event.note_on.note, VoiceAuto,
											  (event.note_on.velocity == 0)? VoiceEvent::Release : VoiceEvent::Create,
											  VoiceEvent::frequency_from_key_id (event.note_on.note, backend()->graph()->master_tune()),
											  event.note_on.velocity / 127.0));
				buffer->push (new VoiceControllerEvent (t, event.note_on.note, event.note_on.velocity / 127.0));
				handled = true;
			}
			break;

		case MIDI::Event::NoteOff:
			if (note_filter && (note_channel == 0 || note_channel == event.note_off.channel + 1))
			{
				buffer->push (new VoiceControllerEvent (t, event.note_off.note, event.note_off.velocity / 127.0));
				buffer->push (new VoiceEvent (t, event.note_off.note, VoiceAuto, VoiceEvent::Release,
											  VoiceEvent::frequency_from_key_id (event.note_off.note, backend()->graph()->master_tune()),
											  event.note_off.velocity / 127.0));
				handled = true;
			}
			break;

		case MIDI::Event::Controller:
			{
				int value = event.controller.value;
				if (controller_invert)
					value = 127 - value;
				if (controller_filter && (controller_channel == 0 || controller_channel == event.controller.channel + 1) && controller_number == static_cast<int> (event.controller.number))
				{
					buffer->push (new ControllerEvent (t, value / 127.0));
					handled = true;
				}
			}
			break;

		case MIDI::Event::Pitchbend:
			if (pitchbend_filter && (pitchbend_channel == 0 || pitchbend_channel == event.pitchbend.channel + 1))
			{
				buffer->push (new ControllerEvent (t, event.pitchbend.value == 0 ? 0.5 : (event.pitchbend.value + 8192) / 16382.0));
				handled = true;
			}
			break;

		case MIDI::Event::ChannelPressure:
			{
				int value = event.channel_pressure.value;
				if (channel_pressure_invert)
					value = 127 - value;
				if (channel_pressure_filter && (channel_pressure_channel == 0 || channel_pressure_channel == event.channel_pressure.channel + 1))
				{
					buffer->push (new ControllerEvent (t, value / 127.0));
					handled = true;
				}
			}
			break;

		case MIDI::Event::KeyPressure:
			{
				int value = event.key_pressure.value;
				if (key_pressure_invert)
					value = 127 - value;
				if (key_pressure_filter && (key_pressure_channel == 0 || key_pressure_channel == event.key_pressure.channel + 1))
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

} // namespace EventBackendImpl

} // namespace Haruhi

