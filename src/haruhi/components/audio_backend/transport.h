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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__TRANSPORT_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__TRANSPORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_buffer.h>


namespace Haruhi {

namespace AudioBackendImpl {

class Backend;

/**
 * Parent class for audio transport implementations
 * (ALSA, JACK, OSS, etc.)
 *
 * Transport should update graph parameters
 * such like sample-rate or buffer-size (after proper graph lock).
 */
class Transport
{
  public:
	class Port
	{
	  public:
		Port (Transport* transport);

		// FIXME change to "= default" in new GCC.
		virtual ~Port() { }

		virtual void
		rename (std::string const&) = 0;

		Transport*
		transport();

		/**
		 * Returns audio buffer to use for transporting audio.
		 */
		virtual AudioBuffer*
		buffer();

	  private:
		Transport*	_transport;
		AudioBuffer	_buffer;
	};

  public:
	Transport (Backend* backend);

	// FIXME change to "= default" in new GCC.
	virtual ~Transport() { }

	Backend*
	backend() const;

	/**
	 * Connects to transport.
	 * \param	client_name is client name for transports
	 * 			that support it. May be ignored.
	 * \throws	AudioBackendException when problem arises.
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
	 * Activates transport (enables periodic callbacks).
	 */
	virtual void
	activate() = 0;

	/**
	 * Deactivates transport (disable periodic callbacks).
	 */
	virtual void
	deactivate() = 0;

	/**
	 * Returns true if transport is active.
	 */
	virtual bool
	active() const = 0;

	/**
	 * Locks ports so no data is copied between them and audio-subsystem.
	 */
	virtual void
	lock_ports() = 0;

	/**
	 * Unlocks ports. Complementary to lock_ports().
	 */
	virtual void
	unlock_ports() = 0;

	/**
	 * Call blocks until transport fetches data from ports.
	 */
	virtual void
	data_ready() = 0;

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

  private:
	Backend* _backend;
};


inline
Transport::Port::Port (Transport* transport):
	_transport (transport),
	_buffer()
{ }


inline Transport*
Transport::Port::transport()
{
	return _transport;
}


inline AudioBuffer*
Transport::Port::buffer()
{
	return &_buffer;
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

} // namespace AudioBackendImpl

} // namespace Haruhi

#endif

