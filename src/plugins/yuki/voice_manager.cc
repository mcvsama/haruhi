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
#include <algorithm>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/memory.h>
#include <haruhi/utility/work_performer.h>

// Local:
#include "voice_manager.h"


namespace Yuki {

VoiceManager::RenderWorkUnit::RenderWorkUnit (Voice* voice, SharedResourcesVec& resources_vec):
	_voice (voice),
	_resources_vec (resources_vec)
{ }


void
VoiceManager::RenderWorkUnit::execute()
{
	_voice->render (_resources_vec[thread_id()]);
}


void
VoiceManager::RenderWorkUnit::mix_result (Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2) const
{
	_voice->mix_result (output_1, output_2);
}


VoiceManager::VoiceManager (Params::Part* part_params, WorkPerformer* work_performer):
	_work_performer (work_performer),
	_sample_rate (0),
	_buffer_size (0),
	_active_voices_number (0),
	_max_polyphony (0)
{
	for (unsigned int i = 0; i < _work_performer->threads_number(); ++i)
		_shared_resources_vec.push_back (new Voice::SharedResources());
}


VoiceManager::~VoiceManager()
{
	kill_voices();

	for (unsigned int i = 0; i < _work_performer->threads_number(); ++i)
		delete _shared_resources_vec[i];
}


void
VoiceManager::handle_voice_event (Haruhi::VoiceEvent const* event)
{
	if (event->type() == Haruhi::VoiceEvent::Create)
	{
		// If there is already voice with the same voice_id, kill it first.
		// This is not a normal situation, so killing instead of dropping is OK.
		delete find_voice_by_id (event->voice_id());

		Haruhi::VoiceID id = event->voice_id();
		Voice* v = new Voice (id, event->timestamp(), _part_params, event->value(), event->frequency() / _sample_rate, _sample_rate, _buffer_size);
		_voices_by_id[id] = _voices.insert (v).first;
		_active_voices_number++;

		check_polyphony_limit();
	}
	// FIXME Perhaps we should have only Release event in Graph?
	else if (event->type() == Haruhi::VoiceEvent::Release || event->type() == Haruhi::VoiceEvent::Drop)
	{
		Voice* v = find_voice_by_id (event->voice_id());
		if (v)
		{
			v->drop();
			_active_voices_number--;
		}
	}
}


void
VoiceManager::panic()
{
	for (Voices::iterator v = _voices.begin(); v != _voices.end(); ++v)
		(*v)->drop();
}


void
VoiceManager::graph_updated (unsigned int sample_rate, std::size_t buffer_size)
{
	_sample_rate = sample_rate;
	_buffer_size = buffer_size;

	_output_1.resize (buffer_size);
	_output_2.resize (buffer_size);

	for (SharedResourcesVec::iterator s = _shared_resources_vec.begin(); s != _shared_resources_vec.end(); ++s)
		(*s)->graph_updated (sample_rate, buffer_size);

	for (Voices::iterator v = _voices.begin(); v != _voices.end(); ++v)
		(*v)->graph_updated (sample_rate, buffer_size);
}


void
VoiceManager::render()
{
	assert (_work_units.empty());

	for (Voices::iterator v = _voices.begin(); v != _voices.end(); ++v)
		_work_units.push_back (new RenderWorkUnit (*v, _shared_resources_vec));

	for (WorkUnits::size_type i = 0, n = _work_units.size(); i < n; ++i)
		_work_performer->add (_work_units[i]);
}


void
VoiceManager::wait_for_render()
{
	_output_1.clear();
	_output_2.clear();

	for (WorkUnits::size_type i = 0, n = _work_units.size(); i < n; ++i)
	{
		_work_units[i]->wait();
		_work_units[i]->mix_result (&_output_1, &_output_2);
	}

	std::for_each (_work_units.begin(), _work_units.end(), delete_operator<WorkPerformer::Unit*>);
	_work_units.clear();
}


void
VoiceManager::mix_rendering_result (Haruhi::AudioBuffer* b1, Haruhi::AudioBuffer* b2)
{
	assert (b1 != 0);
	assert (b2 != 0);

	b1->add (&_output_1);
	b2->add (&_output_2);

	for (Voices::iterator v = _voices.begin(); v != _voices.end(); )
	{
		if ((*v)->state() == Voice::Finished)
		{
			_voices_by_id.erase ((*v)->id());
			delete *v;
			_voices.erase (v++);
		}
		else
			++v;
	}
}


void
VoiceManager::check_polyphony_limit()
{
	while (_active_voices_number > _max_polyphony)
	{
		// Select oldest Voice and drop it:
		Voice* oldest = 0;
		for (Voices::iterator v = _voices.begin(); v != _voices.end(); ++v)
		{
			if ((*v)->state() == Voice::Dropped || (*v)->state() == Voice::Finished)
				continue;
			oldest = oldest
				? Voice::return_older (*v, oldest)
				: *v;
		}
		oldest->drop();
		_active_voices_number--;
	}
}


Voice*
VoiceManager::find_voice_by_id (Haruhi::VoiceID id)
{
	ID2VoiceMap::iterator k = _voices_by_id.find (id);
	if (k != _voices_by_id.end())
		return *k->second;
	return 0;
}


void
VoiceManager::kill_voices()
{
	std::for_each (_voices.begin(), _voices.end(), delete_operator<Voice*>);
	_voices.clear();
	_voices_by_id.clear();
}

} // namespace Yuki

