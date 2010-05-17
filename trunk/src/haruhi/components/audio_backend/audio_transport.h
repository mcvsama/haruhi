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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__AUDIO_TRANSPORT_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__AUDIO_TRANSPORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/core/core.h>
#include <haruhi/core/audio_buffer.h>


namespace Haruhi {

class AudioBackend;

/**
 * Parent class for audio transport implementations
 * (ALSA, JACK, OSS, etc.)
 */
class AudioTransport
{
  public:
	class Port
	{
	  public:
		Port (AudioTransport* transport):
			_transport (transport)
		{ }

		virtual ~Port() { }

		virtual void
		rename (std::string const&) = 0;

		AudioTransport*
		transport() { return _transport; }

		/**
		 * Returns audio buffer to use
		 * for transporting audio.
		 */
		Core::AudioBuffer&
		buffer() { return _buffer; }

		Core::AudioBuffer const&
		buffer() const { return _buffer; }

	  private:
		AudioTransport*		_transport;
		Core::AudioBuffer	_buffer;
	};

  public:
	AudioTransport (AudioBackend* backend):
		_backend (backend)
	{ }

	virtual ~AudioTransport() { }

	AudioBackend*
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

  private:
	AudioBackend* _backend;
};

} // namespace Haruhi

#endif

