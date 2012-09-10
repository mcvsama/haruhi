/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <haruhi/utility/amplitude.h>
#include <haruhi/utility/frequency.h>

// Local:
#include "voice_manager.h"


namespace Yuki {

POOL_ALLOCATOR_FOR (VoiceManager::RenderWorkUnit)


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


VoiceManager::VoiceManager (Params::Main* main_params, Params::Part* part_params, WorkPerformer* work_performer):
	_work_performer (work_performer),
	_main_params (main_params),
	_part_params (part_params),
	_sample_rate (0_Hz),
	_buffer_size (0),
	_oversampling (1),
	_wavetable (0),
	_active_voices_number (0),
	_last_voice_frequency (440_Hz) // Initially concert A.
{
	for (unsigned int i = 0; i < _work_performer->threads_number(); ++i)
		_shared_resources_vec.push_back (new Voice::SharedResources());

	for (AntialiasingFilter& filter: _antialiasing_filter_1)
		filter.assign_impulse_response (&_antialiasing_filter_ir);
	for (AntialiasingFilter& filter: _antialiasing_filter_2)
		filter.assign_impulse_response (&_antialiasing_filter_ir);
	set_oversampling (main_params->oversampling.get());
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
	if (event->action() == Haruhi::VoiceEvent::Action::Create)
	{
		Haruhi::VoiceID id = event->voice_id();

		// If there is already voice with the same voice_id, ignore the event.
		if (!find_voice_by_id (id))
		{
			// For glide effect to work, let's set initial voice frequency to the last
			// voice pitch event we got:
			NormalizedFrequency initial_frequency = _last_voice_frequency / _sample_rate;

			Voice* v = new Voice (id, event->timestamp(), _main_params, _part_params, (0_dB).factor(), initial_frequency, _sample_rate, _buffer_size, _oversampling);
			v->set_wavetable (_wavetable);

			_voices_by_id[id] = _voices.insert (v).first;
			_active_voices_number++;

			check_polyphony_limit();
		}
	}
	else if (event->action() == Haruhi::VoiceEvent::Action::Drop)
	{
		Voice* v = find_voice_by_id (event->voice_id());
		if (v && v->state() == Voice::Voicing)
		{
			v->drop();
			_active_voices_number--;
		}
	}
}


void
VoiceManager::handle_amplitude_event (Haruhi::VoiceControllerEvent const* event)
{
	if (Voice* v = find_voice_by_id (event->voice_id()))
		v->set_amplitude (Amplitude (event->value()));
}


void
VoiceManager::handle_frequency_event (Haruhi::VoiceControllerEvent const* event)
{
	if (Voice* v = find_voice_by_id (event->voice_id()))
		v->set_frequency (event->frequency());
	_last_voice_frequency = event->frequency();
}


void
VoiceManager::panic()
{
	for (Voice* v: _voices)
		v->drop();
}


void
VoiceManager::graph_updated (Frequency sample_rate, std::size_t buffer_size)
{
	_sample_rate = sample_rate;
	_buffer_size = buffer_size;

	resize_buffers();

	for (Voice::SharedResources* s: _shared_resources_vec)
		s->graph_updated (sample_rate, buffer_size);

	for (Voice* v: _voices)
		v->graph_updated (sample_rate, buffer_size);
}


void
VoiceManager::set_oversampling (unsigned int oversampling)
{
	assert (oversampling >= 1);

	_oversampling = oversampling;

	resize_buffers();

	for (Voice::SharedResources* s: _shared_resources_vec)
		s->set_oversampling (_oversampling);

	for (Voice* v: _voices)
		v->set_oversampling (_oversampling);

	// TODO replace with proper 4 or more pole antialiasing filter:
	_antialiasing_filter_ir.set_type (FilterImpulseResponse::LowPass);
	_antialiasing_filter_ir.set_frequency (0.4f / _oversampling);
	_antialiasing_filter_ir.set_resonance (0.8f);
	_antialiasing_filter_ir.set_gain (0.0f);
	_antialiasing_filter_ir.set_attenuation (1.0f);
	_antialiasing_filter_ir.set_limiter_enabled (false);
}


void
VoiceManager::set_wavetable (DSP::Wavetable* wavetable)
{
	_wavetable = wavetable;

	for (Voice* v: _voices)
		v->set_wavetable (_wavetable);
}


void
VoiceManager::render()
{
	assert (_work_units.empty());

	for (Voice* v: _voices)
		_work_units.push_back (new RenderWorkUnit (v, _shared_resources_vec));

	for (WorkUnits::size_type i = 0, n = _work_units.size(); i < n; ++i)
		_work_performer->add (_work_units[i]);
}


void
VoiceManager::wait_for_render()
{
	if (_oversampling == 1)
	{
		_output_1.clear();
		_output_2.clear();

		for (WorkUnits::size_type i = 0, n = _work_units.size(); i < n; ++i)
		{
			_work_units[i]->wait();
			_work_units[i]->mix_result (&_output_1, &_output_2);
		}
	}
	else
	{
		_output_1_oversampled.clear();
		_output_2_oversampled.clear();

		for (WorkUnits::size_type i = 0, n = _work_units.size(); i < n; ++i)
		{
			_work_units[i]->wait();
			_work_units[i]->mix_result (&_output_1_oversampled, &_output_2_oversampled);
		}

		// Antialiasing filtering:
		Haruhi::AudioBuffer* s1 = &_output_1_oversampled;
		Haruhi::AudioBuffer* t1 = &_output_1_filtered;
		Haruhi::AudioBuffer* s2 = &_output_2_oversampled;
		Haruhi::AudioBuffer* t2 = &_output_2_filtered;
		for (std::size_t i = 0; i < countof (_antialiasing_filter_1); ++i)
		{
			_antialiasing_filter_1[i].transform (s1->begin(), s1->end(), t1->begin());
			std::swap (s1, t1);
		}
		for (std::size_t i = 0; i < countof (_antialiasing_filter_1); ++i)
		{
			_antialiasing_filter_2[i].transform (s2->begin(), s2->end(), t2->begin());
			std::swap (s2, t2);
		}

		// Downsample:
		for (Haruhi::AudioBuffer::size_type i = 0; i < _output_1.size(); ++i)
			_output_1[i] = _output_1_filtered[i * _oversampling];
		for (Haruhi::AudioBuffer::size_type i = 0; i < _output_2.size(); ++i)
			_output_2[i] = _output_2_filtered[i * _oversampling];
	}

	std::for_each (_work_units.begin(), _work_units.end(), delete_operator<WorkPerformer::Unit*>);
	_work_units.clear();
}


void
VoiceManager::mix_rendering_result (Haruhi::AudioBuffer* b1, Haruhi::AudioBuffer* b2) noexcept
{
	assert (b1 != 0);
	assert (b2 != 0);
	assert (b1->size() == _output_1.size());
	assert (b2->size() == _output_2.size());

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
VoiceManager::update_voice_parameter (Haruhi::VoiceID voice_id, Params::Voice::ControllerParamPtr param_ptr, int value)
{
	if (voice_id == Haruhi::OmniVoice)
	{
		for (Voice* v: _voices)
			(v->params()->*param_ptr).set (value);
	}
	else
	{
		Voice* v = find_voice_by_id (voice_id);
		if (v)
			(v->params()->*param_ptr).set (value);
	}
}


void
VoiceManager::resize_buffers()
{
	_output_1.resize (_buffer_size);
	_output_2.resize (_buffer_size);
	_output_1_oversampled.resize (_buffer_size * _oversampling);
	_output_2_oversampled.resize (_buffer_size * _oversampling);
	_output_1_filtered.resize (_buffer_size * _oversampling);
	_output_2_filtered.resize (_buffer_size * _oversampling);
}


void
VoiceManager::check_polyphony_limit()
{
	unsigned int max_polyphony = _main_params->polyphony.get();

	while (_active_voices_number > max_polyphony)
	{
		// Select oldest Voice and drop it:
		Voice* oldest = 0;
		for (Voice* v: _voices)
		{
			if (v->state() == Voice::Dropped || v->state() == Voice::Finished)
				continue;
			oldest = oldest
				? Voice::return_older (v, oldest)
				: v;
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

