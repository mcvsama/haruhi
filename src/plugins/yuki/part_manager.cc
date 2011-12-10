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
#include <haruhi/graph/event_buffer.h>
#include <haruhi/application/services.h>

// Local:
#include "part_manager.h"
#include "part_manager_widget.h"


namespace Yuki {

PartManager::PartManager (Plugin* plugin):
	HasPlugin (plugin)
{
	_audio_out[0] = new Haruhi::AudioPort (plugin, "Output 1", Haruhi::Port::Output, 0, Haruhi::Port::StandardAudio);
	_audio_out[1] = new Haruhi::AudioPort (plugin, "Output 2", Haruhi::Port::Output, 0, Haruhi::Port::StandardAudio);
	_voice_in = new Haruhi::EventPort (plugin, "Voice control", Haruhi::Port::Input, 0, Haruhi::Port::ControlKeyboard);
}


PartManager::~PartManager()
{
	remove_all_parts();
	delete _voice_in;
	delete _audio_out[0];
	delete _audio_out[1];
}


void
PartManager::add_part()
{
	Part* p = new Part (this, Haruhi::Services::hi_priority_work_performer(), &_main_params);
	p->set_id (_id_alloc.allocate_id());
	_parts_mutex.lock();
	_parts.push_back (p);
	_parts_mutex.unlock();
	part_added (p);
}


void
PartManager::remove_part (Part* part)
{
	_parts_mutex.lock();
	_parts.remove (part);
	_parts_mutex.unlock();
	part_removed (part);
	_id_alloc.free_id (part->id());
	delete part;
}


void
PartManager::remove_all_parts()
{
	Parts ps = _parts;
	for (Parts::iterator p = ps.begin(); p != ps.end(); ++p)
		remove_part (*p);
}


void
PartManager::ensure_there_is_at_least_one_part()
{
	if (_parts.empty())
		add_part();
}


void
PartManager::set_part_position (Part* part, unsigned int position)
{
	_parts_mutex.lock();
	assert (position < _parts.size());
	Parts::iterator i = std::find (_parts.begin(), _parts.end(), part);
	Parts::iterator b = _parts.begin();
	std::advance (b, position);
	assert (i != _parts.end());
	assert (b != _parts.end());
	_parts.remove (*i);
	_parts.insert (b, *i);
	_parts_mutex.unlock();
}


void
PartManager::process()
{
	_parts_mutex.lock();

	_voice_in->sync();
	Haruhi::EventBuffer const* buffer = _voice_in->event_buffer();
	bool const enabled = _main_params.enabled.get();

	for (Haruhi::EventBuffer::Events::const_iterator e = buffer->events().begin(); e != buffer->events().end(); ++e)
	{
		if ((*e)->event_type() == Haruhi::Event::VoiceEventType)
		{
			Haruhi::VoiceEvent const* voice_event = static_cast<Haruhi::VoiceEvent const*> (e->get());
			if (enabled || voice_event->type() == Haruhi::VoiceEvent::Release || voice_event->type() == Haruhi::VoiceEvent::Drop)
				for (Parts::iterator p = _parts.begin(); p != _parts.end(); ++p)
					(*p)->handle_voice_event (voice_event);
		}
	}

	for (Parts::iterator p = _parts.begin(); p != _parts.end(); ++p)
	{
		(*p)->process();
		(*p)->render();
	}

	for (Parts::iterator p = _parts.begin(); p != _parts.end(); ++p)
	{
		(*p)->wait_for_render();
		(*p)->mix_rendering_result (_audio_out[0]->audio_buffer(), _audio_out[1]->audio_buffer());
	}

	_parts_mutex.unlock();
}


void
PartManager::panic()
{
	_parts_mutex.lock();
	for (Parts::iterator p = _parts.begin(); p != _parts.end(); ++p)
		(*p)->panic();
	_parts_mutex.unlock();
}


void
PartManager::graph_updated()
{
	_parts_mutex.lock();
	for (Parts::iterator p = _parts.begin(); p != _parts.end(); ++p)
		(*p)->graph_updated();
	_parts_mutex.unlock();
}

} // namespace Yuki

