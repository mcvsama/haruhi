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
 */

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__TRANSPORTS__ALSA_TRANSPORT_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__TRANSPORTS__ALSA_TRANSPORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>

// Libs:
#include <alsa/asoundlib.h>
#include <alsa/seq.h>

// Haruhi:
#include <haruhi/components/event_backend/transport.h>
#include <haruhi/lib/midi.h>


namespace Haruhi {

namespace EventBackendImpl {

/**
 * ALSA event transport.
 */
class AlsaTransport: public Transport
{
  public:
	class ALSAPort: public Port
	{
		friend class AlsaTransport;

	  public:
		enum Direction { Input, Output };

	  public:
		ALSAPort (Transport*, Direction, std::string const& name);

		~ALSAPort();

		int
		alsa_port() const { return _alsa_port; }

		void
		rename (std::string const&);

	  private:
		/**
		 * Creates actual ALSA port.
		 * Done in constructor.
		 */
		void
		reinit();

		/**
		 * Destroys ALSA port.
		 */
		void
		destroy();

	  private:
		Direction	_direction;
		std::string	_name;
		int			_alsa_port;
	};

	friend class ALSAPort;

  private:
	typedef std::map<int, ALSAPort*> Ports;

  public:
	AlsaTransport (Backend* backend);

	~AlsaTransport();

	/**
	 * Low-level access to ALSA sequencer.
	 */
	snd_seq_t*
	seq() const { return _seq; }

	/*
	 * Transport API
	 */

	void
	connect (std::string const& client_name);

	void
	disconnect();

	bool
	connected() const;

	Port*
	create_input (std::string const& port_name);

	Port*
	create_output (std::string const& port_name);

	void
	destroy_port (Port*);

	void
	sync();

  private:
	/**
	 * Returns true if event is recognised/supported,
	 * false otherwise.
	 */
	static bool
	map_alsa_to_internal (MIDI::Event& midi, ::snd_seq_event_t* event);

  private:
	// ALSA sequencer:
	snd_seq_t*	_seq;
	Ports		_ports;
};

} // namespace EventBackendImpl

} // namespace Haruhi

#endif

