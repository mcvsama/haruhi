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
 *
 * NOTES
 *
 * Never call JACK functions when session's graph lock is acquired. This may lead to deadlock
 * when JACK will wait for end of its graph processing, and process() function will wait
 * on session's graph lock.
 */

// Standard:
#include <cstddef>
#include <string>

// System:
#include <signal.h>

// Libs:
#include <jack/jack.h>

// Haruhi:
#include <haruhi/core/graph.h>
#include <haruhi/components/audio_backend/audio_backend.h>
#include <haruhi/components/audio_backend/audio_transport.h>

// Local:
#include "jack_audio_transport.h"


namespace Haruhi {

JackAudioTransport::JackPort::JackPort (AudioTransport* transport, Direction direction, jack_port_t* jack_port):
	Port (transport),
	_direction (direction),
	_jack_port (jack_port)
{
}


Core::Sample*
JackAudioTransport::JackPort::buffer()
{
	// TODO return 0 if jack is offline
	return static_cast<Core::Sample*> (jack_port_get_buffer (_jack_port, transport()->backend()->graph()->buffer_size()));
}


void
JackAudioTransport::JackPort::rename (std::string const& new_name)
{
	jack_port_set_name (_jack_port, new_name.c_str());
}


JackAudioTransport::JackAudioTransport (AudioBackend* backend):
	AudioTransport (backend),
	_jack_client (0),
	_active (false)
{
	ignore_sigpipe();
}


JackAudioTransport::~JackAudioTransport()
{
	disconnect();
}


void
JackAudioTransport::connect (std::string const& client_name)
{
	void* vthis = static_cast<void*> (this);

	if (connected())
		disconnect();

	try {
		if (!(_jack_client = jack_client_open (client_name.c_str(), (jack_options_t)JackNoStartServer, 0)))
			throw AudioBackendException ("could not connect to JACK server - is it running?", __func__);

		if (jack_set_process_callback (_jack_client, s_process, vthis))
			throw AudioBackendException ("could not setup process callback", __func__);

		if (jack_set_sample_rate_callback (_jack_client, s_sample_rate_change, vthis))
			throw AudioBackendException ("could not setup sample rate change callback", __func__);

		if (jack_set_buffer_size_callback (_jack_client, s_buffer_size_change, vthis))
			throw AudioBackendException ("could not setup buffer size change callback", __func__);

		jack_on_shutdown (_jack_client, s_shutdown, vthis);

		c_sample_rate_change (jack_get_sample_rate (_jack_client));
		c_buffer_size_change (jack_get_buffer_size (_jack_client));
	}
	catch (Exception const& e)
	{
		disconnect();
		throw;
	}
}


void
JackAudioTransport::disconnect()
{
	if (_jack_client)
	{
		jack_client_close (_jack_client);
		_jack_client = 0;
	}
}


bool
JackAudioTransport::connected() const
{
	return !!_jack_client;
}


void
JackAudioTransport::activate()
{
	if (connected())
	{
		_active = true;
		jack_activate (_jack_client);
	}
}


void
JackAudioTransport::deactivate()
{
	if (connected())
	{
		jack_deactivate (_jack_client);
		_active = false;
	}
}


JackAudioTransport::Port*
JackAudioTransport::create_input (std::string const& port_name)
{
	jack_port_t* jack_port = jack_port_register (_jack_client, port_name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsTerminal | JackPortIsInput, 0);
	if (jack_port == 0)
		throw AudioBackendPortException ("could not create Jack input port", __func__);
	JackPort* port = new JackPort (this, JackPort::Input, jack_port);
	_inputs[jack_port] = port;
	return port;
}


JackAudioTransport::Port*
JackAudioTransport::create_output (std::string const& port_name)
{
	jack_port_t* jack_port = jack_port_register (_jack_client, port_name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsTerminal | JackPortIsOutput, 0);
	if (jack_port == 0)
		throw AudioBackendPortException ("could not create Jack output port", __func__);
	JackPort* port = new JackPort (this, JackPort::Output, jack_port);
	_outputs[jack_port] = port;
	return port;
}


void
JackAudioTransport::destroy_port (Port* port)
{
	JackPort* jack_port = static_cast<JackPort*> (port);
	_inputs.erase (jack_port->jack_port());
	_outputs.erase (jack_port->jack_port());
	// Unregister Jack port:
	jack_port_unregister (_jack_client, jack_port->jack_port());
	delete port;
}


void
JackAudioTransport::ignore_sigpipe()
{
	sigset_t set;
	sigemptyset (&set);
	sigaddset (&set, SIGPIPE);
	sigprocmask (SIG_BLOCK, &set, 0);
}


int
JackAudioTransport::c_process (jack_nframes_t samples)
{
	backend()->transfer();
	return 0;
}


int
JackAudioTransport::c_sample_rate_change (jack_nframes_t sample_rate)
{
	backend()->graph()->lock();
	backend()->graph()->set_sample_rate (sample_rate);
	backend()->graph()->unlock();
	return 0;
}


int
JackAudioTransport::c_buffer_size_change (jack_nframes_t buffer_size)
{
	backend()->graph()->lock();
	backend()->graph()->set_buffer_size (buffer_size);
	backend()->graph()->unlock();
	return 0;
}


void
JackAudioTransport::c_shutdown()
{
	_jack_client = 0;
	_active = false;
	// TODO Invalidate ports
	backend()->notify_disconnected();
}

} // namespace Haruhi

