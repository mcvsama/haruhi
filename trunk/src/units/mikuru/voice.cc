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
#include <iterator>
#include <algorithm>
#include <set>

// Haruhi:
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/functions.h>
#include <haruhi/utility/confusion.h>

// Local:
#include "types.h"
#include "mikuru.h"
#include "voice.h"
#include "oscillator.h"
#include "filter.h"
#include "waveform.h"
#include "synth_thread.h"
#include "general.h"
#include "double_filter.h"
#include "params.h"
#include "voice_manager.h"
#include "part_filters.h"


namespace MikuruPrivate {

Voice::Voice (VoiceManager* voice_manager, SynthThread* thread, Haruhi::KeyID key_id, Haruhi::VoiceID voice_id, Sample frequency, Sample amplitude, Haruhi::Timestamp timestamp):
	_mikuru (voice_manager->part()->mikuru()),
	_params (*voice_manager->part()->oscillator()->voice_params()),
	_part (voice_manager->part()),
	_voice_manager (voice_manager),
	_synth_thread (0),
	_commons (0),
	_key_id (key_id),
	_voice_id (voice_id),
	_timestamp (timestamp),
	_frequency (frequency),
	_target_frequency (frequency),
	_frequency_change (1.0f),
	_amplitude (amplitude),
	_oscillator(),
	_double_filter (_mikuru),
	_last_pitchbend_value (1.0f),
	_attack_sample (0),
	_attack_samples (0.0025f * _mikuru->graph()->sample_rate()), // 2.5ms
	_released (false),
	_drop_sample (0),
	_drop_samples (0.0025f * _mikuru->graph()->sample_rate()), // 2.5ms
	_dropped (false),
	_tracked (false)
{
	set_thread (thread);

	Params::Oscillator* op = _part->oscillator()->oscillator_params();

	_oscillator.set_frequency_source (&_commons->frequency_buffer);
	_oscillator.set_amplitude_source (&_commons->amplitude_buffer);
	_oscillator.set_phase (op->phase.to_f());
	_oscillator.set_initial_phases_spread (_params.unison_init.to_f());

	// Copy current filter parameters:
	_filter1_params = *_part->filters()->filter1()->params();
	_filter2_params = *_part->filters()->filter2()->params();

	int sample_rate = _mikuru->graph()->sample_rate();

	// Setup smoothers:
	_smoother_amplitude.set_smoothing_samples (0.001f * op->amplitude_smoothing.get() * sample_rate);
	_smoother_frequency.set_smoothing_samples (0.001f * op->frequency_smoothing.get() * sample_rate);
	_smoother_pitchbend.set_smoothing_samples (0.001f * op->pitchbend_smoothing.get() * sample_rate);
	_smoother_panorama_1.set_smoothing_samples (0.001f * op->panorama_smoothing.get() * sample_rate);
	_smoother_panorama_2.set_smoothing_samples (0.001f * op->panorama_smoothing.get() * sample_rate);
}


Voice::~Voice()
{
	set_thread (0);
}


void
Voice::set_thread (SynthThread* thread)
{
	if (_synth_thread)
	{
		_commons = 0;
		_synth_thread->voices().erase (this);
		_oscillator.set_frequency_source (0);
		_oscillator.set_amplitude_source (0);
	}
	_synth_thread = thread;
	if (_synth_thread)
	{
		_commons = _synth_thread->voice_commons();
		_synth_thread->voices().insert (this);
		_oscillator.set_frequency_source (&_commons->frequency_buffer);
		_oscillator.set_amplitude_source (&_commons->amplitude_buffer);
	}
}


void
Voice::set_frequency (Sample frequency)
{
	_target_frequency = frequency;
	update_glide_parameters();
}


void
Voice::mixin (Haruhi::AudioBuffer* output1, Haruhi::AudioBuffer* output2)
{
	process_frequency();
	process_amplitude();

	Params::Oscillator* oscillator_params = _part->oscillator()->oscillator_params();

	// Main oscillator:
	_oscillator.set_wavetable (_part->waveform()->wavetable());
	_oscillator.set_unison_spread (2.0f * _params.unison_spread.to_f());
	_oscillator.set_unison_number (_params.unison_index.get());
	_oscillator.set_unison_noise (_params.unison_noise.to_f());
	_oscillator.set_noise_amplitude (oscillator_params->noise_level.to_f());
	_oscillator.set_wavetable_enabled (oscillator_params->wave_enabled.get());
	_oscillator.set_noise_enabled (oscillator_params->noise_enabled.get());
	_oscillator.fill (&_commons->oscillator_buffer);

	_double_filter.configure (static_cast<DoubleFilter::Configuration> (static_cast<int> (_part->filters()->params()->filter_configuration.get())), &_filter1_params, &_filter2_params);
	bool filtered = _double_filter.process (_commons->oscillator_buffer, _commons->filter_buffer1, _commons->filter_buffer2, _commons->output_buffer);
	Haruhi::AudioBuffer& filters_output = filtered ? _commons->output_buffer : _commons->oscillator_buffer;

	// Attacking or dropping? Multiply samples.
	if (_attack_sample < _attack_samples)
	{
		for (unsigned int i = 0; i < filters_output.size() && _attack_sample < _attack_samples; ++i, ++_attack_sample)
			filters_output[i] *= 1.0f * _attack_sample / _attack_samples;
	}
	else if (dropped())
	{
		if (_drop_sample < _drop_samples)
		{
			unsigned int i;
			for (i = 0; i < _commons->output_buffer.size() && _drop_sample < _drop_samples; ++i, ++_drop_sample)
				filters_output[i] *= 1.0 - 1.0f * _drop_sample / _drop_samples;
			std::fill (filters_output.begin() + i, filters_output.end(), 0.0f);
		}
		else
			std::fill (filters_output.begin(), filters_output.end(), 0.0f);
	}

	// Output:
	{
		_commons->output_buffer1.fill (&filters_output);
		_commons->output_buffer2.fill (&filters_output);

		int i = 0;
		float f = 0.0;

		// Panorama:
		i = 0;
		f = 1.0f - 1.0f * _params.panorama.get() / Params::Voice::PanoramaMax;
		f = f > 1.0f ? 1.0 : f;
		_smoother_panorama_1.set_value (f);
		_smoother_panorama_1.multiply (_commons->output_buffer1.begin(), _commons->output_buffer1.end());

		i = 0;
		f = 1.0f - 1.0f * _params.panorama.get() / Params::Voice::PanoramaMin;
		f = f > 1.0f ? 1.0 : f;
		_smoother_panorama_2.set_value (f);
		_smoother_panorama_2.multiply (_commons->output_buffer2.begin(), _commons->output_buffer2.end());

		__brainfuck (",>,>++++++[-<--------<-------->>]", &_commons->output_buffer1, &_commons->output_buffer2);
		__brainfuck ("<<<<++++++[-<++++++++>]<.", &_commons);

		output1->mixin (&_commons->output_buffer1);
		output2->mixin (&_commons->output_buffer2);
	}
}


void
Voice::release()
{
	_released = true;
}


void
Voice::drop()
{
	_dropped = true;
}


bool
Voice::finished() const
{
	return _dropped && _drop_sample == _drop_samples;
}


void
Voice::process_frequency()
{
	Params::Oscillator* const oscillator_params = _part->oscillator()->oscillator_params();
	std::size_t const buffer_size = _commons->output_buffer.size();
	float frequency = 1.0f;

	// Prepare frequency buffer:
	std::fill (_commons->frequency_buffer.begin(), _commons->frequency_buffer.end(), 1.0f);

	// Transposition:
	frequency *= std::pow (2.0f, (1.0f / 12.0f) * oscillator_params->transposition_semitones.get());

	// Glide:
	if (_frequency_change != 1.0f)
	{
		for (Sample *s = _commons->frequency_buffer.begin(), *e = _commons->frequency_buffer.end(); s != e; ++s)
		{
			if ((_frequency_change > 1.0f && _frequency >= _target_frequency) || (_frequency_change < 1.0f && _frequency <= _target_frequency))
				_frequency_change = 1.0f;
			_frequency *= _frequency_change;
			*s *= _frequency;
		}
	}
	else
	{
		_frequency = _target_frequency;
		frequency *= _frequency;
	}

	// Pitchbend:
	{
		float pitchbend = 1.0f;
		if (oscillator_params->pitchbend_enabled.get())
		{
			if (released() && !oscillator_params->pitchbend_released.get())
				pitchbend = _last_pitchbend_value;
			else
			{
				pitchbend = _params.pitchbend.to_f();
				float range = std::pow (2.0f, (1.0f / 12.0f) * (pitchbend >= 0 ? oscillator_params->pitchbend_up_semitones.get() : oscillator_params->pitchbend_down_semitones.get()));
				pitchbend = std::pow (range, pitchbend + 1.0f) / range;
				_last_pitchbend_value = pitchbend;
			}
		}
		_smoother_pitchbend.set_value (pitchbend);
		_smoother_pitchbend.multiply (_commons->frequency_buffer.begin(), _commons->frequency_buffer.end());
	}

	// Frequency modulation and detune:
	{
		float const frq_mod = _params.frequency.to_f();
		float const frq_det = _params.detune.to_f() + _mikuru->general()->params()->detune.to_f();
		_smoother_frequency.set_value (frq_mod);
		_smoother_frequency.fill (_commons->temp1.begin(), _commons->temp1.end());

		Sample* tb = _commons->temp1.begin();
		Sample* fb = _commons->frequency_buffer.begin();
		float range = 1.0f * oscillator_params->frequency_mod_range.get();

		for (std::size_t i = 0; i < buffer_size; ++i)
			fb[i] *= std::pow (2.0f, (1.0f / 12.0f) * (frq_det + tb[i] * range));
	}

	// Multiply buffer by static frequency value:
	for (Sample *s = _commons->frequency_buffer.begin(), *e = _commons->frequency_buffer.end(); s != e; ++s)
		*s *= frequency;
}


void
Voice::process_amplitude()
{
	Params::Oscillator* const oscillator_params = _part->oscillator()->oscillator_params();
	float amplitude = 1.0f;

	// Prepare amplitude buffer:
	std::fill (_commons->amplitude_buffer.begin(), _commons->amplitude_buffer.end(), 1.0f);

	// Amplitude velocity sensing:
	float s = _params.velocity_sens.to_f();
	amplitude *= (s >= 0.0 ? 1 - s + _amplitude * s : s * _amplitude + 1.0f);

	// Volume and amplitude modulation:
	_smoother_amplitude.set_value (oscillator_params->volume.to_f() * _params.amplitude.to_f() * _params.adsr.to_f());
	_smoother_amplitude.multiply (_commons->amplitude_buffer.begin(), _commons->amplitude_buffer.end());

	for (Sample *s = _commons->amplitude_buffer.begin(), *e = _commons->amplitude_buffer.end(); s != e; ++s)
		*s = std::pow (*s, M_E);
}


void
Voice::update_glide_parameters()
{
	float source_frequency = _frequency;

	int portamento_time = _part->oscillator()->oscillator_params()->portamento_time.get();
	if (portamento_time > 0)
	{
		if (_part->oscillator()->oscillator_params()->const_portamento_time.get())
		{
			_frequency_change = std::pow (_target_frequency / source_frequency,
										  1.0f / (1.0f / Params::Oscillator::PortamentoTimeDenominator * portamento_time * _mikuru->graph()->sample_rate()));
		}
		else
		{
			// 2 octaves per portamento time:
			Sample difference = _target_frequency - source_frequency > 0 ? 2.0f : 0.5f;
			_frequency_change = std::pow (difference,
										  1.0f / (1.0f / Params::Oscillator::PortamentoTimeDenominator * portamento_time * _mikuru->graph()->sample_rate()));
		}
	}
	else
		_frequency_change = 1.0;
}

} // namespace MikuruPrivate

