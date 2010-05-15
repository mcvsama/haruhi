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
#include <string>

// Libs:
#include <alsa/asoundlib.h>
#include <alsa/seq.h>

// Haruhi:
#include <haruhi/components/event_backend/event_backend.h>

// Local:
#include "alsa_event_transport.h"


namespace Haruhi {

ALSAEventTransport::ALSAPort::ALSAPort (EventTransport* transport, Direction direction, int alsa_port):
	Port (transport),
	_direction (direction),
	_alsa_port (alsa_port)
{
}


void
ALSAEventTransport::ALSAPort::rename (std::string const& new_name)
{
	ALSAEventTransport* alsa_transport = static_cast<ALSAEventTransport*> (transport());
	snd_seq_port_info_t* info = 0;
	try {
		// Rename ALSA port:
		if (snd_seq_port_info_malloc (&info) < 0)
			throw;
		if (snd_seq_get_port_info (alsa_transport->_seq, alsa_port(), info) < 0)
			throw;
		snd_seq_port_info_set_name (info, new_name.c_str());
		if (snd_seq_set_port_info (alsa_transport->_seq, alsa_port(), info) < 0)
			throw;
		snd_seq_port_info_free (info);
	}
	catch (...)
	{
		if (info)
			snd_seq_port_info_free (info);
	}
}


ALSAEventTransport::ALSAEventTransport (EventBackend* backend):
	EventTransport (backend),
	_seq (0)
{ }


ALSAEventTransport::~ALSAEventTransport()
{
	disconnect();
}


void
ALSAEventTransport::connect (std::string const& client_name)
{
	if (snd_seq_open (&_seq, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK))
		throw EventBackendException ("could not open default ALSA midi sequencer", __func__);
	snd_seq_set_client_name (_seq, client_name.c_str());
}


void
ALSAEventTransport::disconnect()
{
	if (_seq)
	{
		snd_seq_close (_seq);
		_seq = 0;
	}
}


bool
ALSAEventTransport::connected() const
{
	return !!_seq;
}


ALSAEventTransport::Port*
ALSAEventTransport::create_input (std::string const& port_name)
{
	// Create ALSA sequencer port:
	int alsa_port = snd_seq_create_simple_port (_seq, port_name.c_str(),
												SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
												SND_SEQ_PORT_TYPE_SYNTHESIZER | SND_SEQ_PORT_TYPE_SOFTWARE);
	if (alsa_port < 0)
		throw EventBackendPortException ("could not create ALSA input port", __func__);
	ALSAPort* port = new ALSAPort (this, ALSAPort::Input, alsa_port);
	_inputs[alsa_port] = port;
	return port;
}


ALSAEventTransport::Port*
ALSAEventTransport::create_output (std::string const& port_name)
{
	// Create ALSA sequencer port:
	int alsa_port = snd_seq_create_simple_port (_seq, port_name.c_str(),
												SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
												SND_SEQ_PORT_TYPE_SYNTHESIZER | SND_SEQ_PORT_TYPE_SOFTWARE);
	if (alsa_port < 0)
		throw EventBackendPortException ("could not create ALSA input port", __func__);
	ALSAPort* port = new ALSAPort (this, ALSAPort::Output, alsa_port);
	_outputs[alsa_port] = port;
	return port;
}


void
ALSAEventTransport::destroy_port (Port* port)
{
	ALSAPort* alsa_port = static_cast<ALSAPort*> (port);
	_inputs.erase (alsa_port->alsa_port());
	_outputs.erase (alsa_port->alsa_port());
	// Unregister ALSA port:
	snd_seq_delete_port (_seq, alsa_port->alsa_port());
	delete port;
}


void
ALSAEventTransport::sync()
{
	::snd_seq_event_t* e = 0;
	Core::Timestamp t = backend()->graph()->timestamp();

	// Clear all input buffers:
	for (PortsMap::iterator p = _inputs.begin(); p != _inputs.end(); ++p)
		p->second->buffer().clear();

	while (::snd_seq_event_input (_seq, &e) >= 0)
	{
		PortsMap::iterator h = _inputs.find (e->dest.port);
		if (h == _inputs.end())
			std::cerr << "Warning: could not find port given by ALSA sequencer (" << static_cast<int> (e->dest.port) << ") — ignoring" << std::endl;
		else
		{
			MidiBuffer& buf = h->second->buffer();
			MidiEvent midi;
			if (map_alsa_to_internal (midi, e))
			{
				midi.timestamp = t;
				buf.push_back (midi);
			}
		}
		::snd_seq_free_event (e);
	}

	// Clear all output buffers:
	for (PortsMap::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
		p->second->buffer().clear();
}


bool
ALSAEventTransport::map_alsa_to_internal (MidiEvent& midi, ::snd_seq_event_t* event)
{
	switch (event->type)
	{
		case SND_SEQ_EVENT_NOTEON:
			midi.type = MidiEvent::NoteOn;
			midi.note_on.channel = event->data.note.channel;
			midi.note_on.note = event->data.note.note;
			midi.note_on.velocity = event->data.note.velocity;
			break;

		case SND_SEQ_EVENT_NOTEOFF:
			midi.type = MidiEvent::NoteOff;
			midi.note_off.channel = event->data.note.channel;
			midi.note_off.note = event->data.note.note;
			midi.note_off.velocity = event->data.note.off_velocity;
			break;

		case SND_SEQ_EVENT_CONTROLLER:
			midi.type = MidiEvent::Controller;
			midi.controller.channel = event->data.control.channel;
			midi.controller.number = event->data.control.param;
			midi.controller.value = event->data.control.value;
			break;

		case SND_SEQ_EVENT_PITCHBEND:
			midi.type = MidiEvent::Pitchbend;
			midi.pitchbend.channel = event->data.control.channel;
			midi.pitchbend.value = event->data.control.value;
			break;

		case SND_SEQ_EVENT_CHANPRESS:
			midi.type = MidiEvent::ChannelPressure;
			midi.channel_pressure.channel = event->data.control.channel;
			midi.channel_pressure.value = event->data.control.value;
			break;

		case SND_SEQ_EVENT_KEYPRESS:
			midi.type = MidiEvent::KeyPressure;
			midi.key_pressure.channel = event->data.note.channel;
			midi.key_pressure.note = event->data.note.note;
			midi.key_pressure.value = event->data.note.velocity;
			break;

		//case SND_SEQ_EVENT_PGMCHANGE:
		//	break;

		default:
			return false;
	}

	return true;
}

} // namespace Haruhi

