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

Part::Part (PartManager* part_manager, WorkPerformer* work_performer):
	_part_manager (part_manager),
	_voice_manager (new VoiceManager (work_performer))
{
	// Initially resize buffers:
	graph_updated();
	_voice_manager->set_max_polyphony (64);
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


void
Part::render()
{
	_voice_manager->render();
}


void
Part::wait_for_render()
{
	_voice_manager->wait_for_render();
}


void
Part::mix_rendering_result (Haruhi::AudioBuffer* b1, Haruhi::AudioBuffer* b2)
{
	assert (b1 != 0);
	assert (b2 != 0);

	_voice_manager->mix_rendering_result (b1, b2);
}

} // namespace Yuki

