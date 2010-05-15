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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__TRANSPORTS__ALSA_EVENT_TRANSPORT_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__TRANSPORTS__ALSA_EVENT_TRANSPORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>

// Libs:
#include <alsa/asoundlib.h>
#include <alsa/seq.h>

// Haruhi:
#include <haruhi/components/event_backend/event_transport.h>


namespace Haruhi {

/**
 * ALSA event transport.
 */
class ALSAEventTransport: public EventTransport
{
  public:
	class ALSAPort: public Port
	{
	  public:
		enum Direction { Input, Output };

	  public:
		ALSAPort (EventTransport*, Direction, int alsa_port);

		int
		alsa_port() const { return _alsa_port; }

		void
		rename (std::string const&);

	  private:
		Direction	_direction;
		int			_alsa_port;
	};

  private:
	typedef std::map<int, ALSAPort*> PortsMap;

  public:
	ALSAEventTransport (EventBackend* backend);

	~ALSAEventTransport();

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
	map_alsa_to_internal (MidiEvent& midi, ::snd_seq_event_t* event);

  private:
	// ALSA sequencer:
	snd_seq_t*	_seq;
	PortsMap	_inputs;
	PortsMap	_outputs;
};

} // namespace Haruhi

#endif

