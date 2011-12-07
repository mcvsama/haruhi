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


Voice::Voice (Haruhi::VoiceID id, Haruhi::Timestamp timestamp, unsigned int sample_rate, std::size_t buffer_size):
	_id (id),
	_timestamp (timestamp),
	_state (NotStarted)
{
	_state = Voicing;

	graph_updated (sample_rate, buffer_size);
}


void
Voice::drop()
{
	_state = Finished;
}


bool
Voice::render (SharedResources* res)
{
	_output_1[0] = +0.1;
	_output_2[0] = -0.1;
	return true;
}


void
Voice::mix_result (Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2) const
{
	output_1->mixin (&_output_1);
	output_2->mixin (&_output_2);
}


void
Voice::graph_updated (unsigned int, std::size_t buffer_size)
{
	_output_1.resize (buffer_size);
	_output_2.resize (buffer_size);
}

} // namespace Yuki

