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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__TRANSPORTS__JACK_AUDIO_TRANSPORT_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__TRANSPORTS__JACK_AUDIO_TRANSPORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>

// Libs:
#include <jack/jack.h>

// Haruhi:
#include <haruhi/components/audio_backend/audio_transport.h>


namespace Haruhi {

/**
 * JACK audio transport.
 */
class JackAudioTransport: public AudioTransport
{
  public:
	/**
	 * JackPort can be online (when JACK is connected),
	 * or offline (JACK not connected).
	 */
	class JackPort: public Port
	{
		friend class JackAudioTransport;

	  public:
		enum Direction { Input, Output };

	  public:
		/**
		 * Creates JackPort. Also initializes JACK port with reinit().
		 */
		JackPort (AudioTransport*, Direction, std::string const& name);

		/**
		 * Destroys JACK port using destroy().
		 */
		virtual ~JackPort();

		Core::Sample*
		buffer();

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

	  private:
		Direction		_direction;
		std::string		_name;
		jack_port_t*	_jack_port;
	};

	friend class JackPort;

  private:
	typedef std::set<JackPort*> Ports;

  public:
	JackAudioTransport (AudioBackend* backend);

	~JackAudioTransport();

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

	bool
	active() const { return _active; }

	Port*
	create_input (std::string const& port_name);

	Port*
	create_output (std::string const& port_name);

	void
	destroy_port (Port*);

  private:
	jack_client_t*
	jack_client() const { return _jack_client; }

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
		{ return reinterpret_cast<JackAudioTransport*> (klass)->c_process (samples); }

	static int
	s_sample_rate_change (jack_nframes_t sample_rate, void* klass)
		{ return reinterpret_cast<JackAudioTransport*> (klass)->c_sample_rate_change (sample_rate); }

	static int
	s_buffer_size_change (jack_nframes_t buffer_size, void* klass)
		{ return reinterpret_cast<JackAudioTransport*> (klass)->c_buffer_size_change (buffer_size); }

	static void
	s_shutdown (void* klass)
	 	{ reinterpret_cast<JackAudioTransport*> (klass)->c_shutdown(); }

  private:
	jack_client_t*	_jack_client;
	Ports			_ports;
	bool			_active;
};

} // namespace Haruhi

#endif

