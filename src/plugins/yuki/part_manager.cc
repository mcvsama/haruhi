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

PartManager::MainPorts::MainPorts (Plugin* plugin):
	HasPlugin (plugin)
{
	if (graph())
		graph()->lock();

	audio_out[0]	= new Haruhi::AudioPort (plugin, "Output 1", Haruhi::Port::Output, 0, Haruhi::Port::StandardAudio);
	audio_out[1]	= new Haruhi::AudioPort (plugin, "Output 2", Haruhi::Port::Output, 0, Haruhi::Port::StandardAudio);

	voice_in		= new Haruhi::EventPort (plugin, "Voice control", Haruhi::Port::Input, 0, Haruhi::Port::ControlVoice);

	volume			= new Haruhi::EventPort (plugin, "Volume", Haruhi::Port::Input);
	panorama		= new Haruhi::EventPort (plugin, "Panorama", Haruhi::Port::Input);
	detune			= new Haruhi::EventPort (plugin, "Detune", Haruhi::Port::Input);
	stereo_width	= new Haruhi::EventPort (plugin, "Stereo width", Haruhi::Port::Input);

	amplitude		= new Haruhi::EventPort (plugin, "Amplitude", Haruhi::Port::Input, 0, Haruhi::Port::Polyphonic);
	frequency		= new Haruhi::EventPort (plugin, "Frequency", Haruhi::Port::Input, 0, Haruhi::Port::Polyphonic);
	pitchbend		= new Haruhi::EventPort (plugin, "Pitchbend", Haruhi::Port::Input, 0, Haruhi::Port::Polyphonic | Haruhi::Port::ControlPitchbend);

	if (graph())
		graph()->unlock();
}


PartManager::MainPorts::~MainPorts()
{
	// TODO is checking for graph() needed?
	if (graph())
		graph()->unlock();
	delete audio_out[0];
	delete audio_out[1];
	delete voice_in;
	delete volume;
	delete panorama;
	delete detune;
	delete stereo_width;
	delete amplitude;
	delete frequency;
	delete pitchbend;
	if (graph())
		graph()->unlock();
}


PartManager::MainProxies::MainProxies (MainPorts* main_ports, Params::Main* main_params):
#define CONSTRUCT_CONTROLLER_PROXY(name) name (main_ports->name, &main_params->name)
	CONSTRUCT_CONTROLLER_PROXY (volume),
	CONSTRUCT_CONTROLLER_PROXY (panorama),
	CONSTRUCT_CONTROLLER_PROXY (detune),
	CONSTRUCT_CONTROLLER_PROXY (stereo_width)
#undef CONSTRUCT_CONTROLLER_PROXY
{ }


void
PartManager::MainProxies::process_events()
{
#define PROXY_PROCESS_EVENTS(name) name.process_events();
	PROXY_PROCESS_EVENTS (volume);
	PROXY_PROCESS_EVENTS (panorama);
	PROXY_PROCESS_EVENTS (detune);
	PROXY_PROCESS_EVENTS (stereo_width);
#undef PROXY_PROCESS_EVENTS
}


PartManager::PartManager (Plugin* plugin):
	HasPlugin (plugin),
	_ports (plugin),
	_proxies (&_ports, &_main_params)
{ }


PartManager::~PartManager()
{
	remove_all_parts();
}


void
PartManager::add_part()
{
	Part* p = new Part (this, Haruhi::Services::hi_priority_work_performer(), &_main_params, _id_alloc.allocate_id());
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
	_proxies.process_events();

	_parts_mutex.lock();

	// Forward all messages from common ports to parts' ports:
	// TODO

	// Send voice events:

	_ports.voice_in->sync();
	Haruhi::EventBuffer const* buffer = _ports.voice_in->event_buffer();
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

	// Render output:

	Haruhi::AudioBuffer* buf_0 = _ports.audio_out[0]->audio_buffer();
	Haruhi::AudioBuffer* buf_1 = _ports.audio_out[1]->audio_buffer();

	for (Parts::iterator p = _parts.begin(); p != _parts.end(); ++p)
	{
		(*p)->process_events();
		(*p)->render();
	}

	for (Parts::iterator p = _parts.begin(); p != _parts.end(); ++p)
	{
		(*p)->wait_for_render();
		(*p)->mix_rendering_result (buf_0, buf_1);
	}

	// Stereo width:

	float w = 1.0f - 0.5f * (1.0f + _main_params.stereo_width.to_f());
	Sample o1, o2;
	for (Sample *s1 = buf_0->begin(), *s2 = buf_1->begin(); s1 != buf_0->end(); ++s1, ++s2)
	{
		float y = _stereo_width_smoother.process (w);
		float x = 1.0f - y;
		o1 = *s1;
		o2 = *s2;
		*s1 = x * *s1 + y * o2;
		*s2 = x * *s2 + y * o1;
	}

	// Panorama:

	float f = 0.0f;
	f = 1.0f - 1.0f / Params::Main::PanoramaMax * _main_params.panorama.get();
	f = f > 1.0f ? 1.0f : f;
	_panorama_smoother[0].multiply (buf_0->begin(), buf_0->end(), f);

	f = 1.0f - 1.0f / Params::Main::PanoramaMin * _main_params.panorama.get();
	f = f > 1.0f ? 1.0f : f;
	_panorama_smoother[1].multiply (buf_1->begin(), buf_1->end(), f);

	// Master volume:

	Haruhi::Sample v = FastPow::pow (_main_params.volume.to_f(), M_E);
	_volume_smoother[0].multiply (buf_0->begin(), buf_0->end(), v);
	_volume_smoother[1].multiply (buf_1->begin(), buf_1->end(), v);

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

	float const samples = 0.005f * graph()->sample_rate();
	_volume_smoother[0].set_samples (samples);
	_volume_smoother[1].set_samples (samples);
	_panorama_smoother[0].set_samples (samples);
	_panorama_smoother[1].set_samples (samples);
	_stereo_width_smoother.set_samples (samples);

	for (Parts::iterator p = _parts.begin(); p != _parts.end(); ++p)
		(*p)->graph_updated();
	_parts_mutex.unlock();
}

} // namespace Yuki

