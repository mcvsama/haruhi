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
 *
 * NOTES
 *
 * Never call JACK functions when graph's lock is acquired. This may lead to deadlock
 * when JACK will wait for end of its graph processing, and process() function will wait
 * on graph's lock.
 *
 * As there is assumption that in processing round graph cannot be modified you should only
 * acquire graph lock when changing core objects (ports, unit attributes, connecting ports, etc.).
 */

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__TRANSPORTS__JACK_TRANSPORT_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__TRANSPORTS__JACK_TRANSPORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <set>

// Libs:
#include <jack/jack.h>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/components/audio_backend/transport.h>
#include <haruhi/utility/semaphore.h>
#include <haruhi/utility/mutex.h>


namespace Haruhi {

namespace AudioBackendImpl {

/**
 * JACK audio transport.
 */
class JackTransport: public Transport
{
  public:
	/**
	 * JackPort can be online (when JACK is connected),
	 * or offline (JACK not connected).
	 */
	class JackPort: public Port
	{
		friend class JackTransport;

	  public:
		enum Direction { Input, Output };

	  public:
		/**
		 * Creates JackPort. Also initializes JACK port with reinit().
		 */
		JackPort (Transport*, Direction, std::string const& name);

		/**
		 * Destroys JACK port using destroy().
		 */
		virtual ~JackPort();

		/**
		 * Copies data from internal buffer to JACK buffer.
		 */
		void
		copy_to_jack();

		/**
		 * Copies data from JACK buffer to internal buffer.
		 */
		void
		copy_from_jack();

		/**
		 * Calls copy_to_jack or copy_from_jack depending on port
		 * direction (Input, Output).
		 */
		void
		transfer_data();

		/*
		 * Transport::Port API
		 */

		void
		rename (std::string const&);

	  private:
		/**
		 * Creates actual JACK port.
		 * Done in constructor.
		 */
		void
		reinit();

		/**
		 * Destroys JACK port.
		 */
		void
		destroy();

		Sample*
		jack_buffer();

	  private:
		Direction		_direction;
		std::string		_name;
		jack_port_t*	_jack_port;
	};

	friend class JackPort;

  private:
	typedef std::set<JackPort*> Ports;

  public:
	JackTransport (Backend* backend);

	~JackTransport();

	/*
	 * Transport API
	 */

	void
	connect (std::string const& client_name);

	void
	disconnect();

	bool
	connected() const;

	void
	activate();

	void
	deactivate();

	void
	lock_ports() { _ports_mutex.lock(); }

	void
	unlock_ports() { _ports_mutex.unlock(); }

	void
	data_ready();

	bool
	active() const { return _active; }

	Port*
	create_input (std::string const& port_name);

	Port*
	create_output (std::string const& port_name);

	void
	destroy_port (Port*);

	jack_client_t*
	jack_client() const { return _jack_client; }

  private:
	/**
	 * Calls post on semaphores to avoid death locks.
	 */
	void
	deactivated();

	/**
	 * Blocks SIGPIPE to avoid terminating program due to failure on JACK read.
	 */
	void
	ignore_sigpipe();

	/**
	 * Main JACK processing callback.
	 */
	int
	c_process (jack_nframes_t samples);

	/**
	 * Called when changing sample rate.
	 */
	int
	c_sample_rate_change (jack_nframes_t sample_rate);

	/**
	 * Called when changing buffer sizes.
	 */
	int
	c_buffer_size_change (jack_nframes_t buffer_size);

	/**
	 * Called when JACKit daemon shuts down.
	 */
	void
	c_shutdown();

	/*
	 * Static callbacks methods that will call their non-static versions.
	 */

	static int
	s_process (jack_nframes_t samples, void* klass)
		{ return reinterpret_cast<JackTransport*> (klass)->c_process (samples); }

	static int
	s_sample_rate_change (jack_nframes_t sample_rate, void* klass)
		{ return reinterpret_cast<JackTransport*> (klass)->c_sample_rate_change (sample_rate); }

	static int
	s_buffer_size_change (jack_nframes_t buffer_size, void* klass)
		{ return reinterpret_cast<JackTransport*> (klass)->c_buffer_size_change (buffer_size); }

	static void
	s_shutdown (void* klass)
	 	{ reinterpret_cast<JackTransport*> (klass)->c_shutdown(); }

	static void
	s_log_error (const char*);

	static void
	s_log_info (const char*);

  private:
	jack_client_t*	_jack_client;
	Ports			_ports;
	bool			_active;
	Mutex			_ports_mutex;
	Semaphore		_data_ready;
};

} // namespace AudioBackendImpl

} // namespace Haruhi

#endif
