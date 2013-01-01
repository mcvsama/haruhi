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
#include <string>

// Libs:
#include <alsa/asoundlib.h>
#include <alsa/seq.h>

// Haruhi:
#include <haruhi/components/event_backend/backend.h>

// Local:
#include "alsa_transport.h"


namespace Haruhi {

namespace EventBackendImpl {

AlsaTransport::AlsaPort::AlsaPort (Transport* transport, Direction direction, std::string const& name):
	Port (transport),
	_direction (direction),
	_name (name),
	_alsa_port (0),
	_alsa_port_info (0)
{
	if (::snd_seq_port_info_malloc (&_alsa_port_info))
		throw;
	reinit();
}


AlsaTransport::AlsaPort::~AlsaPort()
{
	destroy();
	if (_alsa_port_info)
		::snd_seq_port_info_free (_alsa_port_info);
}


void
AlsaTransport::AlsaPort::rename (std::string const& new_name)
{
	_name = new_name;
	if (transport()->connected())
	{
		// These calls may fail, but so what?
		if (snd_seq_get_port_info (alsa_transport()->seq(), _alsa_port, _alsa_port_info) >= 0)
		{
			snd_seq_port_info_set_name (_alsa_port_info, new_name.c_str());
			snd_seq_set_port_info (alsa_transport()->seq(), _alsa_port, _alsa_port_info);
		}
	}
}


int
AlsaTransport::AlsaPort::readers() const
{
	if (snd_seq_get_port_info (alsa_transport()->seq(), _alsa_port, _alsa_port_info) >= 0)
		return snd_seq_port_info_get_read_use (_alsa_port_info);
	return 0;
}


int
AlsaTransport::AlsaPort::writers() const
{
	if (snd_seq_get_port_info (alsa_transport()->seq(), _alsa_port, _alsa_port_info) >= 0)
		return snd_seq_port_info_get_write_use (_alsa_port_info);
	return 0;
}


void
AlsaTransport::AlsaPort::reinit()
{
	if (transport()->connected())
	{
		switch (_direction)
		{
			case Input:
				_alsa_port = snd_seq_create_simple_port (static_cast<AlsaTransport*> (transport())->seq(), _name.c_str(),
														 SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
														 SND_SEQ_PORT_TYPE_SYNTHESIZER | SND_SEQ_PORT_TYPE_SOFTWARE);
				break;
			case Output:
				_alsa_port = snd_seq_create_simple_port (static_cast<AlsaTransport*> (transport())->seq(), _name.c_str(),
														 SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
														 SND_SEQ_PORT_TYPE_SYNTHESIZER | SND_SEQ_PORT_TYPE_SOFTWARE);
				break;
		}
	}
}


void
AlsaTransport::AlsaPort::destroy()
{
	if (transport()->connected())
		snd_seq_delete_port (static_cast<AlsaTransport*> (transport())->seq(), _alsa_port);
}


AlsaTransport::AlsaTransport (Backend* backend):
	Transport (backend),
	_seq (0)
{ }


AlsaTransport::~AlsaTransport()
{
	disconnect();
}


void
AlsaTransport::connect (std::string const& client_name)
{
	if (snd_seq_open (&_seq, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK))
		throw Exception ("could not open default ALSA midi sequencer", __func__);
	// Free some not-sure-what-is-it-for cache allocated implicitly by ALSA, to prevent
	// memory leaks inside alsalib:
	snd_config_update_free_global();
	snd_seq_set_client_name (_seq, client_name.c_str());
	// Switch all ports online:
	for (auto& p: _ports)
		p.second->reinit();
}


void
AlsaTransport::disconnect()
{
	if (_seq)
	{
		for (auto& p: _ports)
			p.second->destroy();
		snd_seq_t* c = _seq;
		_seq = 0;
		snd_seq_close (c);
	}
}


bool
AlsaTransport::connected() const
{
	return !!_seq;
}


AlsaTransport::Port*
AlsaTransport::create_input (std::string const& port_name)
{
	AlsaPort* port = new AlsaPort (this, AlsaPort::Input, port_name);
	_ports[port->alsa_port()] = port;
	return port;
}


AlsaTransport::Port*
AlsaTransport::create_output (std::string const& port_name)
{
	AlsaPort* port = new AlsaPort (this, AlsaPort::Output, port_name);
	_ports[port->alsa_port()] = port;
	return port;
}


void
AlsaTransport::destroy_port (Port* port)
{
	_ports.erase (static_cast<AlsaPort*> (port)->alsa_port());
	delete port;
}


void
AlsaTransport::sync()
{
	if (!connected())
		return;

	::snd_seq_event_t* e = 0;
	Timestamp t = backend()->graph()->timestamp();

	// Clear all buffers:
	for (auto& p: _ports)
		p.second->buffer().clear();

	while (::snd_seq_event_input (_seq, &e) >= 0)
	{
		Ports::iterator h = _ports.find (e->dest.port);
		if (h == _ports.end())
			std::cerr << "Warning: could not find port given by ALSA sequencer (" << static_cast<int> (e->dest.port) << ") — ignoring" << std::endl;
		else
		{
			MidiBuffer& buf = h->second->buffer();
			MIDI::Event midi;
			if (map_alsa_to_internal (midi, e))
			{
				midi.timestamp = t;
				buf.push_back (midi);
			}
		}
		::snd_seq_free_event (e);
	}
}


bool
AlsaTransport::learning_possible() const
{
	for (auto& p: _ports)
		if (p.second->writers() > 0)
			return true;
	return false;
}


bool
AlsaTransport::map_alsa_to_internal (MIDI::Event& midi, ::snd_seq_event_t* event)
{
	switch (event->type)
	{
		case SND_SEQ_EVENT_NOTEON:
			// Some keyboards send NOTEON with velocity 0 instead of NOTEOFF:
			if (event->data.note.velocity > 0)
			{
				midi.type = MIDI::Event::NoteOn;
				midi.note_on.channel = event->data.note.channel;
				midi.note_on.note = event->data.note.note;
				midi.note_on.velocity = event->data.note.velocity;
			}
			else
			{
				midi.type = MIDI::Event::NoteOff;
				midi.note_off.channel = event->data.note.channel;
				midi.note_off.note = event->data.note.note;
				midi.note_off.velocity = 0;
			}
			break;

		case SND_SEQ_EVENT_NOTEOFF:
			midi.type = MIDI::Event::NoteOff;
			midi.note_off.channel = event->data.note.channel;
			midi.note_off.note = event->data.note.note;
			midi.note_off.velocity = event->data.note.off_velocity;
			break;

		case SND_SEQ_EVENT_CONTROLLER:
			midi.type = MIDI::Event::Controller;
			midi.controller.channel = event->data.control.channel;
			midi.controller.number = event->data.control.param;
			midi.controller.value = event->data.control.value;
			break;

		case SND_SEQ_EVENT_PITCHBEND:
			midi.type = MIDI::Event::Pitchbend;
			midi.pitchbend.channel = event->data.control.channel;
			midi.pitchbend.value = event->data.control.value;
			break;

		case SND_SEQ_EVENT_CHANPRESS:
			midi.type = MIDI::Event::ChannelPressure;
			midi.channel_pressure.channel = event->data.control.channel;
			midi.channel_pressure.value = event->data.control.value;
			break;

		case SND_SEQ_EVENT_KEYPRESS:
			midi.type = MIDI::Event::KeyPressure;
			midi.key_pressure.channel = event->data.note.channel;
			midi.key_pressure.note = event->data.note.note;
			midi.key_pressure.value = event->data.note.velocity;
			break;

		default:
			return false;
	}

	return true;
}

} // namespace EventBackendImpl

} // namespace Haruhi

