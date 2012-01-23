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

#ifndef HARUHI__GRAPH__AUDIO_PORT_H__INCLUDED
#define HARUHI__GRAPH__AUDIO_PORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/noncopyable.h>

// Local:
#include "port.h"
#include "port_group.h"
#include "audio_buffer.h"


namespace Haruhi {

class AudioPort: public Port
{
  public:
	/**
	 * Locks Graph for operation - it is safe to create new AudioPort
	 * without need to explicitly locking Graph before.
	 * \entry	Any thread.
	 */
	AudioPort (Unit* unit, std::string const& name, Port::Direction direction, PortGroup* group = 0, Flags flags = 0);

	/**
	 * Locks Graph for operation - it is safe to delete AudioPort
	 * without need to explicitly locking Graph before.
	 * \entry	Any thread
	 */
	~AudioPort();

	/**
	 * Helper that casts Buffer to AudioBuffer.
	 */
	AudioBuffer*
	audio_buffer() const;

	/**
	 * Implementation of Port::graph_updated().
	 */
	void
	graph_updated();
};


inline AudioBuffer*
AudioPort::audio_buffer() const
{
	return static_cast<AudioBuffer*> (buffer());
}

} // namespace Haruhi

#endif

