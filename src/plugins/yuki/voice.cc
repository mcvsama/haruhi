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

// Local:
#include "voice.h"


namespace Yuki {

void
Voice::SharedResources::graph_updated (unsigned int, std::size_t buffer_size)
{
	output_1.resize (buffer_size);
	output_2.resize (buffer_size);
}


Voice::Voice (Haruhi::VoiceID id, Haruhi::Timestamp timestamp):
	_id (id),
	_timestamp (timestamp),
	_state (NotStarted)
{
	_state = Voicing;
}


void
Voice::drop()
{
	_state = Finished;
}


bool
Voice::render (SharedResources* res)
{
	return false;
}


void
Voice::graph_updated (unsigned int sample_rate, std::size_t buffer_size)
{
}

} // namespace Yuki

