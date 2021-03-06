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

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/utility/confusion.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/simd_ops.h>
#include <haruhi/utility/amplitude.h>

// Local:
#include "voice.h"


namespace Yuki {

constexpr Time	Voice::AttackTime;
constexpr Time	Voice::DropTime;


void
Voice::SharedResources::graph_updated (Frequency, std::size_t buffer_size)
{
	_buffer_size = buffer_size;
	resize_buffers();
}


void
Voice::SharedResources::set_oversampling (unsigned int oversampling)
{
	_oversampling = oversampling;
	resize_buffers();
}


void
Voice::SharedResources::resize_buffers()
{
	amplitude_buf.resize (_buffer_size * _oversampling);
	frequency_buf.resize (_buffer_size * _oversampling);
	fm_buf.resize (_buffer_size * _oversampling);
	for (Haruhi::AudioBuffer& buf: tmp_buf)
		buf.resize (_buffer_size * _oversampling);
}


Voice::Voice (Haruhi::VoiceID id, Time timestamp, Params::Main* main_params, Params::Part* part_params,
			  Amplitude amplitude, NormalizedFrequency frequency, Frequency sample_rate, std::size_t buffer_size, unsigned int oversampling):
	_id (id),
	_timestamp (timestamp),
	_state (Voicing),
	_params (part_params->voice),
	_part_params (part_params),
	_main_params (main_params),
	_amplitude (amplitude),
	_frequency (frequency),
	_sample_rate (sample_rate),
	_buffer_size (buffer_size),
	_oversampling (oversampling),
	_vmod (part_params, buffer_size),
	_dual_filter (&_params.filters[0], &_params.filters[1]),
	_target_frequency (frequency),
	_frequency_change (0.0f),
	_attack_sample (0),
	_drop_sample (0),
	_first_pass (true)
{
	resize_buffers();

	_vosc.set_phase (_part_params->phase.to_f());
	_vosc.set_initial_phases_spread (_params.unison_init.to_f());

	// This will call recompute_sampling_rate_dependents();
	set_oversampling (_oversampling);

	update_glide_parameters();
}


bool
Voice::render (SharedResources* res)
{
	if (_state == Finished)
		return false;

	prepare_amplitude_buffer (&res->amplitude_buf);
	prepare_frequency_buffer (&res->frequency_buf, res->tmp_buf + 0);

	// Apply modulation:
	res->fm_buf.fill (1.0f);
	if (_part_params->modulator_enabled)
		_vmod.modulate (&res->amplitude_buf, &res->frequency_buf, &res->fm_buf, res->tmp_buf);

	// Generate oscillation:
	_vosc.set_amplitude_source (&res->amplitude_buf);
	_vosc.set_frequency_source (&res->frequency_buf);
	_vosc.set_fm_source (&res->fm_buf);
	_vosc.set_unison_spread (_params.unison_spread.to_f());
	_vosc.set_unison_number (_params.unison_index.get());
	_vosc.set_unison_noise (_params.unison_noise.to_f());
	_vosc.set_unison_stereo (!!_part_params->unison_stereo.get());
	_vosc.set_unison_vibrato_level (_params.unison_vibrato_level.to_f());
	_vosc.set_unison_vibrato_frequency (1_Hz * _params.unison_vibrato_frequency.to_f() / _sample_rate / _oversampling); // Max 10 Hz
	_vosc.set_noise_amplitude (Amplitude (_part_params->noise_level.to_f()));
	_vosc.set_wave_enabled (_part_params->wave_enabled.get());
	_vosc.set_noise_enabled (_part_params->noise_enabled.get());
	_vosc.fill (&_output_1, &_output_2);

	// Filter:
	_dual_filter.configure (static_cast<DualFilter::Configuration> (_part_params->filter_configuration.get()), _sample_rate);
	bool filtered = _dual_filter.process (&_output_1, &_output_2, res->tmp_buf + 0, res->tmp_buf + 1, res->tmp_buf + 2, res->tmp_buf + 3);
	Haruhi::AudioBuffer* filters_output_1 = filtered ? res->tmp_buf + 2 : &_output_1;
	Haruhi::AudioBuffer* filters_output_2 = filtered ? res->tmp_buf + 3 : &_output_2;

	// Smooth Attacking or dropping:
	if (_attack_sample < _attack_samples)
	{
		// Voice phase in:
		for (std::size_t i = 0; i < _buffer_size * _oversampling && _attack_sample < _attack_samples; ++i, ++_attack_sample)
		{
			float const k = 1.0f * _attack_sample / _attack_samples;
			(*filters_output_1)[i] *= k;
			(*filters_output_2)[i] *= k;
		}
	}
	else if (_state == Dropped)
	{
		if (_drop_sample < _drop_samples)
		{
			// Voice phase out:
			std::size_t i;
			for (i = 0; i < _buffer_size * _oversampling && _drop_sample < _drop_samples; ++i, ++_drop_sample)
			{
				float const k = 1.0f - 1.0f * _drop_sample / _drop_samples;
				(*filters_output_1)[i] *= k;
				(*filters_output_2)[i] *= k;
			}
			// Starting point is not 16-byte aligned, can't use SIMD operations:
			std::fill (filters_output_1->begin() + i, filters_output_1->end(), 0.0f);
			std::fill (filters_output_2->begin() + i, filters_output_2->end(), 0.0f);
		}
		else
		{
			_state = Finished;
			_output_1.clear();
			_output_2.clear();
			return false;
		}
	}

	// Panorama:
	{
		float f = 0.0;

		f = 1.0f - 1.0f / Params::Voice::PanoramaMax * _params.panorama.get();
		f = f > 1.0f ? 1.0 : f;
		if (_first_pass)
			_smoother_panorama_1.reset (f);
		_smoother_panorama_1.multiply (filters_output_1->begin(), filters_output_1->end(), f);

		f = 1.0f - 1.0f / Params::Voice::PanoramaMin * _params.panorama.get();
		f = f > 1.0f ? 1.0 : f;
		if (_first_pass)
			_smoother_panorama_2.reset (f);
		_smoother_panorama_2.multiply (filters_output_2->begin(), filters_output_2->end(), f);

		__brainfuck (",>,>++++++[-<--------<-------->>]", &res->output_1, &res->output_2);
		__brainfuck ("<<<<++++++[-<++++++++>]<.", &res);
	}

	// Final output:
	if (filters_output_1 != &_output_1)
		_output_1.fill (filters_output_1);
	if (filters_output_2 != &_output_2)
		_output_2.fill (filters_output_2);

	_first_pass = false;
	return true;
}


void
Voice::graph_updated (Frequency sample_rate, std::size_t buffer_size)
{
	_sample_rate = sample_rate;
	_buffer_size = buffer_size;

	_vmod.graph_updated (buffer_size);
	resize_buffers();

}


void
Voice::set_oversampling (unsigned int oversampling)
{
	float change_factor = static_cast<float> (oversampling) / _oversampling;
	_oversampling = oversampling;

	// Update some sample rate dependent variables to correctly
	// reflect position in buffer:
	_attack_sample *= change_factor;
	_drop_sample *= change_factor;

	_vmod.set_oversampling (oversampling);
	_dual_filter.set_oversampling (oversampling);
	resize_buffers();
	recompute_sampling_rate_dependents();
}


void
Voice::resize_buffers()
{
	_output_1.resize (_buffer_size * _oversampling);
	_output_2.resize (_buffer_size * _oversampling);
}


void
Voice::recompute_sampling_rate_dependents() noexcept
{
	_attack_samples = AttackTime * _sample_rate * _oversampling;
	_drop_samples = DropTime * _sample_rate * _oversampling;

	// Setup smoothers for 5ms/50ms. Response time must be independent from sample rate.
	_smoother_amplitude.set_samples (5_ms * _sample_rate * _oversampling);
	_smoother_frequency.set_samples (5_ms * _sample_rate * _oversampling);
	_smoother_pitchbend.set_samples (50_ms * _sample_rate * _oversampling);
	_smoother_panorama_1.set_samples (5_ms * _sample_rate * _oversampling);
	_smoother_panorama_2.set_samples (5_ms * _sample_rate * _oversampling);
}


void
Voice::update_glide_parameters() noexcept
{
	Time portamento_time = 1_s * _part_params->portamento_time.get();

	if (portamento_time > 0_s)
	{
		if (_part_params->const_portamento_time.get())
		{
			_frequency_change = std::pow (_target_frequency / _frequency,
										  1.0f / (1.0f / Params::Part::PortamentoTimeDenominator * portamento_time * _sample_rate * _oversampling));
		}
		else
		{
			// 2 octaves per portamento time:
			Sample difference = _target_frequency > _frequency ? 2.0f : 0.5f;
			_frequency_change = std::pow (difference,
										  1.0f / (1.0f / Params::Part::PortamentoTimeDenominator * portamento_time * _sample_rate * _oversampling));
		}
	}
	else
		_frequency_change = 1.0f;
}


void
Voice::prepare_amplitude_buffer (Haruhi::AudioBuffer* buffer) noexcept
{
	// Amplitude velocity sensing:
	float sens = _params.velocity_sens.to_f();
	sens = (sens >= 0.0 ? 1 - sens + _amplitude * sens : sens * _amplitude + 1.0f);
	float f = sens * _part_params->volume.to_f() * _params.amplitude.to_f();

	if (_first_pass)
		_smoother_amplitude.reset (f);
	// Volume and amplitude modulation:
	_smoother_amplitude.fill (buffer->begin(), buffer->end(), f);

	SIMD::power_buffer_to_scalar (buffer->begin(), buffer->size(), M_E);
}


void
Voice::prepare_frequency_buffer (Haruhi::AudioBuffer* buffer, Haruhi::AudioBuffer* tmp_buf) noexcept
{
	buffer->fill (1.0f / _oversampling);

	// Transposition:
	float frequency = FastPow::pow_radix_2 ((1.0f / 12.0f) * _part_params->transposition_semitones.get());

	// Glide:
	if (_frequency_change != 1.0f)
	{
		if (_frequency_change > 1.0f)
		{
			for (Sample& s: *buffer)
			{
				if (_frequency >= _target_frequency)
					_frequency_change = 1.0f;
				_frequency *= _frequency_change;
				s *= _frequency;
			}
		}
		else if (_frequency_change < 1.0f)
		{
			for (Sample& s: *buffer)
			{
				if (_frequency <= _target_frequency)
					_frequency_change = 1.0f;
				_frequency *= _frequency_change;
				s *= _frequency;
			}
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
		if (_part_params->pitchbend_enabled.get())
		{
			pitchbend = _params.pitchbend.to_f();
			float range = FastPow::pow_radix_2 ((1.0f / 12.0f) * (pitchbend >= 0 ? _part_params->pitchbend_up_semitones.get() : _part_params->pitchbend_down_semitones.get()));
			pitchbend = FastPow::pow (range, pitchbend + 1.0f) / range;
			_last_pitchbend_value = pitchbend;
		}
		if (_first_pass)
			_smoother_pitchbend.reset (pitchbend);
		_smoother_pitchbend.multiply (buffer->begin(), buffer->end(), pitchbend);
	}

	// Frequency modulation and detune:
	{
		float const frq_mod = _params.frequency.to_f();
		float const frq_det = _params.detune.to_f() + _main_params->detune.to_f();
		if (_first_pass)
			_smoother_frequency.reset (frq_mod);
		_smoother_frequency.fill (tmp_buf->begin(), tmp_buf->end(), frq_mod);

		Sample* tb = tmp_buf->begin();
		Sample* fb = buffer->begin();
		float range = 1.0f * _part_params->frequency_mod_range.get();
		std::size_t nsamples = _buffer_size * _oversampling;

#if defined(HARUHI_SSE1) && defined(HARUHI_HAS_SSE_POW)
		// 2 times faster on Core2 than scalar code using SSEPow, but
		// only 1.3 times faster on Core2 than scalar code using LookupPow :/
		__m128 range4 = _mm_set_ps1 (range);
		__m128 frq_det4 = _mm_set_ps1 (frq_det);
		__m128 t;
		__m128* tp = reinterpret_cast<__m128*> (tb);
		__m128* fp = reinterpret_cast<__m128*> (fb);

		for (std::size_t i = 0; i < nsamples; i += 4, ++tp, ++fp)
		{
			t = _mm_mul_ps (*tp, range4);					// tb[i] * range
			t = _mm_add_ps (t, frq_det4);					// frq_det + tb[i] * range
			t = _mm_mul_ps (t, _mm_set_ps1 (1.0f / 12.0f));	// (1.0f/12.0f) + (frq_det + tb[i] * range)
			t = SSEPow::vec4_pow_radix_2 (t);				// 2^…
			*fp = _mm_mul_ps (*fp, t);
		}

		// The rest:
		for (std::size_t i = nsamples / 4 * 4; i < nsamples; ++i)
			fb[i] *= FastPow::pow_radix_2 ((1.0f / 12.0f) * (frq_det + tb[i] * range));
#else
		// Generic code:
		for (std::size_t i = 0; i < nsamples; ++i)
			fb[i] *= FastPow::pow_radix_2 ((1.0f / 12.0f) * (frq_det + tb[i] * range));
#endif
	}

	// Multiply buffer by frequency value:
	buffer->attenuate (frequency);
}

} // namespace Yuki

