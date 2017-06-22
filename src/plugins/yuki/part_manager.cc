/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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
#include <functional>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/event_buffer.h>
#include <haruhi/application/services.h>
#include <haruhi/utility/qdom.h>

// Local:
#include "part_manager.h"
#include "part_manager_widget.h"


namespace Yuki {

PartManager::MainPorts::MainPorts (Plugin* plugin):
	HasPlugin (plugin)
{
	audio_out[0]	= std::make_unique<Haruhi::AudioPort> (plugin, "Output 1", Haruhi::Port::Output, nullptr, Haruhi::Port::StandardAudio, Haruhi::AudioPort::Tags { "audio-1" });
	audio_out[1]	= std::make_unique<Haruhi::AudioPort> (plugin, "Output 2", Haruhi::Port::Output, nullptr, Haruhi::Port::StandardAudio, Haruhi::AudioPort::Tags { "audio-2" });

	voice_in		= std::make_unique<Haruhi::EventPort> (plugin, "Voice control", Haruhi::Port::Input, nullptr, 0, Haruhi::AudioPort::Tags { "voice" });
	voice_pitch		= std::make_unique<Haruhi::EventPort> (plugin, "Voice pitch", Haruhi::Port::Input, nullptr, Haruhi::Port::Polyphonic, Haruhi::AudioPort::Tags { "voice-pitch" });
	voice_velocity	= std::make_unique<Haruhi::EventPort> (plugin, "Voice velocity", Haruhi::Port::Input, nullptr, Haruhi::Port::Polyphonic, Haruhi::AudioPort::Tags { "voice-velocity" });

	volume			= std::make_unique<Haruhi::EventPort> (plugin, "Level", Haruhi::Port::Input);
	panorama		= std::make_unique<Haruhi::EventPort> (plugin, "Panorama", Haruhi::Port::Input);
	detune			= std::make_unique<Haruhi::EventPort> (plugin, "Detune", Haruhi::Port::Input);
	stereo_width	= std::make_unique<Haruhi::EventPort> (plugin, "Stereo width", Haruhi::Port::Input);

	amplitude		= std::make_unique<Haruhi::EventPort> (plugin, "Mod. amplitude", Haruhi::Port::Input, nullptr, Haruhi::Port::Polyphonic);
	frequency		= std::make_unique<Haruhi::EventPort> (plugin, "Mod. frequency", Haruhi::Port::Input, nullptr, Haruhi::Port::Polyphonic);
	pitchbend		= std::make_unique<Haruhi::EventPort> (plugin, "Mod. pitchbend", Haruhi::Port::Input, nullptr, Haruhi::Port::Polyphonic, Haruhi::AudioPort::Tags { "pitchbend" });

	amplitude->set_default_value (1.0);
	frequency->set_default_value (1.0);
	pitchbend->set_default_value (0.5);
}


PartManager::MainProxies::MainProxies (MainPorts* main_ports, Params::Main* main_params):
#define CONSTRUCT_CONTROLLER_PROXY(name) name (main_ports->name.get(), &main_params->name)
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
{
	_main_params.oversampling.on_change.connect (this, &PartManager::oversampling_updated);
}


PartManager::~PartManager()
{
	remove_all_parts();
}


Part*
PartManager::add_part()
{
	Part* p = new Part (this, Haruhi::Services::hi_priority_work_performer(), &_main_params, _id_alloc.allocate_id());
	auto graph_lock = get_graph_lock();
	_parts_mutex.synchronize ([&] {
		_parts.push_back (p);
	});
	part_added (p);
	return p;
}


void
PartManager::remove_part (Part* part)
{
	{
		auto graph_lock = get_graph_lock();

		_parts_mutex.synchronize ([&] {
			_parts.remove (part);
		});
		part_removed (part);
	}

	_id_alloc.free_id (part->id());
	delete part;
}


void
PartManager::remove_all_parts()
{
	for (Part* p: Parts (_parts))
		remove_part (p);
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
	Mutex::Lock lock (_parts_mutex);
	assert (position < _parts.size());
	Parts::iterator i = std::find (_parts.begin(), _parts.end(), part);
	assert (i != _parts.end());
	_parts.remove (*i);
	Parts::iterator b = _parts.begin();
	std::advance (b, position);
	_parts.insert (b, *i);
}


void
PartManager::set_part_position (unsigned int old_position, unsigned int new_position)
{
	Mutex::Lock lock (_parts_mutex);
	assert (old_position < _parts.size());
	assert (new_position < _parts.size());
	Parts::iterator o = _parts.begin();
	std::advance (o, old_position);
	set_part_position (*o, new_position);
}


void
PartManager::process()
{
	_proxies.process_events();

	Mutex::Lock lock (_parts_mutex);

	// Distribute voice events to all Parts:

	_ports.voice_in->sync();
	bool const enabled = _main_params.enabled.get();

	// VoiceEvents:
	for (auto e: _ports.voice_in->buffer()->events())
	{
		if (e->event_type() == Haruhi::Event::VoiceEventType)
		{
			Haruhi::VoiceEvent const* ev = static_cast<Haruhi::VoiceEvent const*> (e.get());
			if (enabled || ev->action() == Haruhi::VoiceEvent::Action::Drop)
				for (Part* p: _parts)
					p->handle_voice_event (ev);
		}
	}

	if (enabled)
	{
		// Pitch (frequency) events:
		_ports.voice_pitch->sync();
		for (auto e: _ports.voice_pitch->buffer()->events())
		{
			if (e->event_type() == Haruhi::Event::VoiceControllerEventType)
			{
				Haruhi::VoiceControllerEvent const* ev = static_cast<Haruhi::VoiceControllerEvent const*> (e.get());
				for (Part* p: _parts)
					p->handle_frequency_event (ev);
			}
		}

		// Velocity (amplitude) events:
		_ports.voice_velocity->sync();
		for (auto e: _ports.voice_velocity->buffer()->events())
		{
			if (e->event_type() == Haruhi::Event::VoiceControllerEventType)
			{
				Haruhi::VoiceControllerEvent const* ev = static_cast<Haruhi::VoiceControllerEvent const*> (e.get());
				for (Part* p: _parts)
					p->handle_amplitude_event (ev);
			}
		}
	}

	// Render output:

	Haruhi::AudioBuffer* buf_0 = _ports.audio_out[0]->buffer();
	Haruhi::AudioBuffer* buf_1 = _ports.audio_out[1]->buffer();

	for (Part* p: _parts)
	{
		p->process_events();
		p->async_render();
	}

	for (Part* p: _parts)
	{
		p->wait_for_render();
		p->mix_rendering_result (buf_0, buf_1);
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
}


void
PartManager::panic()
{
	Mutex::Lock lock (_parts_mutex);
	for (Part* p: _parts)
		p->panic();
}


void
PartManager::graph_updated()
{
	// Graph lock not needed, this method is called from Graph when it's already locked.

	float const samples = 5_ms * graph()->sample_rate();
	_volume_smoother[0].set_samples (samples);
	_volume_smoother[1].set_samples (samples);
	_panorama_smoother[0].set_samples (samples);
	_panorama_smoother[1].set_samples (samples);
	_stereo_width_smoother.set_samples (samples);

	Parts parts_copy;
	_parts_mutex.synchronize ([&] {
		parts_copy = _parts;
	});

	for (Part* p: parts_copy)
		p->graph_updated();
}


unsigned int
PartManager::voices_number() const
{
	Mutex::Lock lock (_parts_mutex);
	unsigned int sum = 0;
	for (Part* p: _parts)
		sum += p->voices_number();
	return sum;
}


void
PartManager::save_state (QDomElement& element) const
{
	auto graph_lock = get_graph_lock();
	auto lock = _parts_mutex.get_lock();

	QDomElement e = element.ownerDocument().createElement ("main");
	_main_params.save_state (e);
	element.appendChild (e);

	for (Part* p: _parts)
	{
		QDomElement e = element.ownerDocument().createElement ("part");
		p->save_state (e);
		e.setAttribute ("id", p->id());
		element.appendChild (e);
	}
}


void
PartManager::load_state (QDomElement const& element)
{
	auto graph_lock = get_graph_lock();
	auto parts_lock = _parts_mutex.get_lock();

	remove_all_parts();

	for (QDomElement& e: element)
	{
		if (e.tagName() == "part")
		{
			Part* p = add_part();

			unsigned int loaded_id = e.attribute ("id", "0").toUInt();
			_id_alloc.free_id (p->id());
			_id_alloc.reserve_id (loaded_id);

			p->load_state (e);
			p->set_id (loaded_id);

			part_updated (p);
		}
		else if (e.tagName() == "main")
			_main_params.load_state (e);
	}
}


void
PartManager::oversampling_updated()
{
	Haruhi::Services::call_out (std::bind (&PartManager::set_oversampling, this, _main_params.oversampling.get()));
}


void
PartManager::set_oversampling (unsigned int oversampling)
{
	auto graph_lock = get_graph_lock();

	for (Part* p: _parts)
		p->set_oversampling (oversampling);
}


Mutex::Lock
PartManager::get_graph_lock() const
{
	if (graph())
		return graph()->get_lock();
	return Mutex::Lock();
}

} // namespace Yuki

