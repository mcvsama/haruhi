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
#include <haruhi/utility/memory.h>

// Local:
#include "voice_manager.h"


namespace Yuki {

VoiceManager::VoiceManager (Plugin* plugin):
	HasPlugin (plugin),
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
VoiceManager::graph_updated()
{
	unsigned int bs = graph()->buffer_size();
	_tmpbuf1.resize (bs);
	_tmpbuf2.resize (bs);

	for (Voices::iterator v = _voices.begin(); v != _voices.end(); ++v)
		(*v)->graph_updated();
}


void
VoiceManager::render (Haruhi::AudioBuffer* b1, Haruhi::AudioBuffer* b2)
{
	b1->clear();
	b2->clear();

	for (Voices::iterator v = _voices.begin(); v != _voices.end(); )
	{
		if ((*v)->render (&_tmpbuf1, &_tmpbuf2))
		{
			b1->add (&_tmpbuf1);
			b2->add (&_tmpbuf2);
		}

		if ((*v)->state() == Voice::Finished)
		{
			_voices_by_id.erase ((*v)->id());
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

} // namespace Yuki

