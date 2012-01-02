/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
	class AlsaPort: public Port
	{
		friend class AlsaTransport;

	  public:
		enum Direction { Input, Output };

	  public:
		AlsaPort (Transport*, Direction, std::string const& name);

		~AlsaPort();

		int
		alsa_port() const;

		void
		rename (std::string const&);

		/**
		 * Returns the number of ALSA clients listening on this port.
		 */
		int
		readers() const;

		/**
		 * Returns the number of ALSA clients writing to this port.
		 */
		int
		writers() const;

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

		AlsaTransport*
		alsa_transport() const;

	  private:
		Direction				_direction;
		std::string				_name;
		int						_alsa_port;
		snd_seq_port_info_t*	_alsa_port_info;
	};

	friend class AlsaPort;

  private:
	typedef std::map<int, AlsaPort*> Ports;

  public:
	AlsaTransport (Backend* backend);

	~AlsaTransport();

	/**
	 * Low-level access to ALSA sequencer.
	 */
	snd_seq_t*
	seq() const;

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

	bool
	learning_possible() const;

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


inline int
AlsaTransport::AlsaPort::alsa_port() const
{
	return _alsa_port;
}


inline AlsaTransport*
AlsaTransport::AlsaPort::alsa_transport() const
{
	return static_cast<AlsaTransport*> (transport());
}


inline snd_seq_t*
AlsaTransport::seq() const
{
	return _seq;
}

} // namespace EventBackendImpl

} // namespace Haruhi

#endif

