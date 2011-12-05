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

// Lib:
#include <boost/bind.hpp>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/memory.h>
#include <haruhi/utility/work_performer.h>

// Local:
#include "voice_manager.h"


namespace Yuki {

VoiceManager::VoiceManager():
	_active_voices_number (0),
	_max_polyphony (0)
{ }


VoiceManager::~VoiceManager()
{
	kill_voices();
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
		_voices_by_id[id] = _voices.insert (new Voice (id, event->timestamp())).first;
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
	_tmp_voice_buf1.resize (buffer_size);
	_tmp_voice_buf2.resize (buffer_size);
	_tmp_mixed_buf1.resize (buffer_size);
	_tmp_mixed_buf2.resize (buffer_size);

	for (Voices::iterator v = _voices.begin(); v != _voices.end(); ++v)
		(*v)->graph_updated (sample_rate, buffer_size);
}


void
VoiceManager::render (WorkPerformer* work_performer)
{
	assert (_work_units.empty());

	_tmp_mixed_buf1.clear();
	_tmp_mixed_buf2.clear();

	for (Voices::iterator v = _voices.begin(); v != _voices.end(); ++v)
	{
		WorkPerformer::Unit* wu = WorkPerformer::make_unit (boost::bind (VoiceManager::render_voice, *v,
																		 &_tmp_voice_buf1, &_tmp_voice_buf2,
																		 &_tmp_mixed_buf1, &_tmp_mixed_buf2));
		_work_units.push_back (wu);
	}

	for (WorkUnits::size_type i = 0, n = _work_units.size(); i < n; ++i)
		work_performer->add (_work_units[i]);
}


void
VoiceManager::wait_for_render()
{
	for (WorkUnits::size_type i = 0, n = _work_units.size(); i < n; ++i)
		_work_units[i]->wait();

	std::for_each (_work_units.begin(), _work_units.end(), delete_operator<WorkPerformer::Unit*>);
	_work_units.clear();
}


void
VoiceManager::mix_rendering_result (Haruhi::AudioBuffer* b1, Haruhi::AudioBuffer* b2)
{
	assert (b1 != 0);
	assert (b2 != 0);

	b1->add (&_tmp_mixed_buf1);
	b2->add (&_tmp_mixed_buf2);

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
			if (!(*v)->state() == Voice::Dropped)
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


void
VoiceManager::render_voice (Voice* voice,
							Haruhi::AudioBuffer* tmp1, Haruhi::AudioBuffer* tmp2,
							Haruhi::AudioBuffer* mix1, Haruhi::AudioBuffer* mix2)
{
	if (voice->render (tmp1, tmp2))
	{
		mix1->add (tmp1);
		mix2->add (tmp2);
	}
}

} // namespace Yuki

