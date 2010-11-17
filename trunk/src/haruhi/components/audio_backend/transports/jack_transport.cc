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
#include <iostream>

// System:
#include <signal.h>

// Libs:
#include <jack/jack.h>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/graph.h>
#include <haruhi/components/audio_backend/backend.h>
#include <haruhi/components/audio_backend/transport.h>

// Local:
#include "jack_transport.h"


namespace Haruhi {

namespace AudioBackendImpl {

JackTransport::JackPort::JackPort (Transport* transport, Direction direction, std::string const& name):
	Port (transport),
	_direction (direction),
	_name (name),
	_jack_port (0)
{
	jack_set_error_function (JackTransport::s_log_error);
	jack_set_info_function (JackTransport::s_log_info);
	reinit();
}


JackTransport::JackPort::~JackPort()
{
	destroy();
}


Sample*
JackTransport::JackPort::buffer()
{
	if (transport()->connected())
		return static_cast<Sample*> (jack_port_get_buffer (_jack_port, transport()->backend()->graph()->buffer_size()));
	return 0;
}


void
JackTransport::JackPort::rename (std::string const& new_name)
{
	_name = new_name;
	if (transport()->connected())
		jack_port_set_name (_jack_port, new_name.c_str());
}


void
JackTransport::JackPort::reinit()
{
	if (transport()->connected())
	{
		switch (_direction)
		{
			case Input:
				_jack_port = jack_port_register (static_cast<JackTransport*> (transport())->jack_client(), _name.c_str(),
												 JACK_DEFAULT_AUDIO_TYPE, JackPortIsTerminal | JackPortIsInput, 0);
				break;
			case Output:
				_jack_port = jack_port_register (static_cast<JackTransport*> (transport())->jack_client(), _name.c_str(),
												 JACK_DEFAULT_AUDIO_TYPE, JackPortIsTerminal | JackPortIsOutput, 0);
				break;
		}
	}
}


void
JackTransport::JackPort::destroy()
{
	if (transport()->connected())
		jack_port_unregister (static_cast<JackTransport*> (transport())->jack_client(), _jack_port);
}


JackTransport::JackTransport (Backend* backend):
	Transport (backend),
	_jack_client (0),
	_active (false),
	_wait_for_tick (0),
	_data_ready (0)
{
	ignore_sigpipe();
}


JackTransport::~JackTransport()
{
	disconnect();
}


void
JackTransport::connect (std::string const& client_name)
{
	void* vthis = static_cast<void*> (this);

	if (connected())
		disconnect();

	try {
		if (!(_jack_client = jack_client_open (client_name.c_str(), (jack_options_t)JackNoStartServer, 0)))
			throw Exception ("could not connect to JACK server - is it running?", __func__);

		if (jack_set_process_callback (_jack_client, s_process, vthis))
			throw Exception ("could not setup process callback", __func__);

		if (jack_set_sample_rate_callback (_jack_client, s_sample_rate_change, vthis))
			throw Exception ("could not setup sample rate change callback", __func__);

		if (jack_set_buffer_size_callback (_jack_client, s_buffer_size_change, vthis))
			throw Exception ("could not setup buffer size change callback", __func__);

		jack_on_shutdown (_jack_client, s_shutdown, vthis);

		c_sample_rate_change (jack_get_sample_rate (_jack_client));
		c_buffer_size_change (jack_get_buffer_size (_jack_client));

		// Switch all ports online:
		for (Ports::iterator p = _ports.begin(); p != _ports.end(); ++p)
			(*p)->reinit();
	}
	catch (Exception const& e)
	{
		disconnect();
		throw;
	}
}


void
JackTransport::disconnect()
{
	if (_jack_client)
	{
		deactivate();
		for (Ports::iterator p = _ports.begin(); p != _ports.end(); ++p)
			(*p)->destroy();
		jack_client_t* c = _jack_client;
		_jack_client = 0;
		jack_client_close (c);
	}
}


bool
JackTransport::connected() const
{
	return !!_jack_client;
}


void
JackTransport::activate()
{
	if (connected())
	{
		_active = true;
		jack_activate (_jack_client);
	}
}


void
JackTransport::deactivate()
{
	if (connected())
	{
		// Ensure that Jack is not stuck in process() function:
		_data_ready.post();
		// Deactivate:
		jack_deactivate (_jack_client);
		_active = false;
	}
}


void
JackTransport::wait_for_tick()
{
	_wait_for_tick.wait();
}


void
JackTransport::data_ready()
{
	_data_ready.post();
}


JackTransport::Port*
JackTransport::create_input (std::string const& port_name)
{
	JackPort* port = new JackPort (this, JackPort::Input, port_name);
	_ports.insert (port);
	return port;
}


JackTransport::Port*
JackTransport::create_output (std::string const& port_name)
{
	JackPort* port = new JackPort (this, JackPort::Output, port_name);
	_ports.insert (port);
	return port;
}


void
JackTransport::destroy_port (Port* port)
{
	_ports.erase (static_cast<JackPort*> (port));
	delete port;
}


void
JackTransport::ignore_sigpipe()
{
	sigset_t set;
	sigemptyset (&set);
	sigaddset (&set, SIGPIPE);
	sigprocmask (SIG_BLOCK, &set, 0);
}


int
JackTransport::c_process (jack_nframes_t samples)
{
	_wait_for_tick.post();
	_data_ready.wait();
	return 0;
}


int
JackTransport::c_sample_rate_change (jack_nframes_t sample_rate)
{
	backend()->graph()->lock();
	backend()->graph()->set_sample_rate (sample_rate);
	backend()->graph()->unlock();
	return 0;
}


int
JackTransport::c_buffer_size_change (jack_nframes_t buffer_size)
{
	backend()->graph()->lock();
	backend()->graph()->set_buffer_size (buffer_size);
	backend()->graph()->unlock();
	return 0;
}


void
JackTransport::c_shutdown()
{
	_jack_client = 0;
	_active = false;
	backend()->notify_disconnected();
}


void
JackTransport::s_log_error (const char* message)
{
	std::clog << "ERROR[JACK] " << message << std::endl;
}


void
JackTransport::s_log_info (const char* message)
{
	std::clog << "INFO[JACK] " << message << std::endl;
}

} // namespace AudioBackendImpl

} // namespace Haruhi

