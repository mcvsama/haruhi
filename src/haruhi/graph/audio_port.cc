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

// Standard:
#include <string>

// Local:
#include "audio_port.h"
#include "audio_buffer.h"
#include "graph.h"
#include "unit.h"


namespace Haruhi {

AudioPort::AudioPort (Unit* unit, std::string const& name, Port::Direction direction, PortGroup* group, Flags flags):
	Port (unit, name, direction, new AudioBuffer(), group, flags)
{
	register_me();
}


AudioPort::~AudioPort()
{
	unregister_me();
}


AudioBuffer*
AudioPort::audio_buffer() const
{
	return static_cast<AudioBuffer*> (buffer());
}


void
AudioPort::graph_updated()
{
	audio_buffer()->resize (graph()->buffer_size());
}

} // namespace Haruhi
