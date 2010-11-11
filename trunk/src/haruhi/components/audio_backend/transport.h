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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__TRANSPORT_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__TRANSPORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/core/core.h>
#include <haruhi/core/audio_buffer.h>


namespace Haruhi {

namespace AudioBackend {

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
		Port (Transport* transport):
			_transport (transport),
			_buffer (1)
		{ }

		virtual ~Port() { }

		virtual void
		rename (std::string const&) = 0;

		Transport*
		transport() { return _transport; }

		/**
		 * Returns audio buffer to use for transporting audio.
		 * May return 0 if data is not available/port is disabled.
		 */
		virtual Sample*
		buffer() = 0;

	  private:
		Transport*			_transport;
		Core::AudioBuffer	_buffer;
	};

  public:
	Transport (Backend* backend):
		_backend (backend)
	{ }

	virtual ~Transport() { }

	Backend*
	backend() const { return _backend; }

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

} // namespace AudioBackend

} // namespace Haruhi

#endif

