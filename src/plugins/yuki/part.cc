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
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/event.h>

// Local:
#include "part.h"
#include "part_manager.h"
#include "plugin.h"
#include "voice_manager.h"


namespace Yuki {

Part::Part (PartManager* part_manager):
	_part_manager (part_manager),
	_voice_manager (new VoiceManager())
{
}


Part::~Part()
{
	delete _voice_manager;
}


WaveComputer*
Part::wave_computer() const
{
	return _part_manager->plugin()->wave_computer();
}


void
Part::handle_voice_event (Haruhi::VoiceEvent const* event)
{
	if (_params.enabled.get())
		_voice_manager->handle_voice_event (event);
}


void
Part::process()
{
	// TODO process controllers
}


void
Part::panic()
{
	_voice_manager->panic();
}


void
Part::graph_updated()
{
	Haruhi::Graph* graph = _part_manager->graph();
	_voice_manager->graph_updated (graph->sample_rate(), graph->buffer_size());
}

} // namespace Yuki

