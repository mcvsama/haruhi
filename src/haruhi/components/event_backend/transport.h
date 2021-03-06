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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__TRANSPORT_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__TRANSPORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/lib/midi.h>


namespace Haruhi {

namespace EventBackendImpl {

class Backend;

/**
 * Parent class for event transport implementations
 * (ALSA, JACK, OSS, etc.)
 */
class Transport
{
  public:
	typedef std::list<MIDI::Event> MidiBuffer;

	class Port
	{
	  public:
		Port (Transport* transport);

		virtual ~Port() = default;

		virtual void
		rename (std::string const&) = 0;

		Transport*
		transport() const;

		/**
		 * Returns event buffer to use
		 * for transporting events.
		 */
		MidiBuffer&
		buffer();

		MidiBuffer const&
		buffer() const;

	  private:
		Transport*	_transport;
		MidiBuffer	_buffer;
	};

  public:
	Transport (Backend* backend);

	virtual ~Transport() = default;

	Backend*
	backend() const;

	/**
	 * Connects to transport.
	 * \param	client_name is client name for transports
	 *			that support it. May be ignored.
	 * \throws	Exception when problem arises.
	 */
	virtual void
	connect (std::string const& client_name) = 0;

	/**
	 * Disconnects from transport.
	 */
	virtual void
	disconnect() = 0;

	/**
	 * Returns true if transport is in connected state.
	 */
	virtual bool
	connected() const = 0;

	/**
	 * Creates input port with given name.
	 * Should never return 0, instead it should throw
	 * an exception.
	 */
	virtual Port*
	create_input (std::string const& port_name) = 0;

	/**
	 * Creates output port with given name.
	 * Should never return 0, instead it should throw
	 * an exception.
	 */
	virtual Port*
	create_output (std::string const& port_name) = 0;

	/**
	 * Destroys port.
	 */
	virtual void
	destroy_port (Port*) = 0;

	/**
	 * Synchronizes ports (fills buffers, etc).
	 */
	virtual void
	sync() = 0;

	/**
	 * Returns true if there learning from incoming transport events is possible,
	 * that is at least one MIDI source is connected to any of transport's
	 * ports.
	 */
	virtual bool
	learning_possible() const = 0;

  private:
	Backend* _backend;
};


inline
Transport::Port::Port (Transport* transport):
	_transport (transport)
{ }


inline Transport*
Transport::Port::transport() const
{
	return _transport;
}


inline Transport::MidiBuffer&
Transport::Port::buffer()
{
	return _buffer;
}


inline Transport::MidiBuffer const&
Transport::Port::buffer() const
{
	return _buffer;
}


inline
Transport::Transport (Backend* backend):
	_backend (backend)
{ }


inline Backend*
Transport::backend() const
{
	return _backend;
}

} // namespace EventBackendImpl

} // namespace Haruhi

#endif

