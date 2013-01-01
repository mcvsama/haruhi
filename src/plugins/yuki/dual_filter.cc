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

// Local:
#include "dual_filter.h"


namespace Yuki {

constexpr int DualFilter::MaxStages;


DualFilter::DualFilter (Params::Filter* params_1, Params::Filter* params_2):
	_sample_rate (0_Hz),
	_params_1 (params_1),
	_params_2 (params_2),
	_impulse_response_1 (FilterImpulseResponse::LowPass, 0.0, 0.0, 0.0, 1.0),
	_impulse_response_2 (FilterImpulseResponse::LowPass, 0.0, 0.0, 0.0, 1.0)
{
	// Two channels:
	for (int c = 0; c < 2; ++c)
	{
		// Each stage:
		for (int i = 0; i < MaxStages; ++i)
		{
			_filter_1[c][i].assign_impulse_response (&_impulse_response_1);
			_filter_2[c][i].assign_impulse_response (&_impulse_response_2);
		}
	}
}


void
DualFilter::reset()
{
	// Two channels:
	for (int c = 0; c < 2; ++c)
	{
		// Each stage:
		for (int i = 0; i < MaxStages; ++i)
		{
			_filter_1[c][i].reset();
			_filter_2[c][i].reset();
		}
	}
}


void
DualFilter::configure (Configuration configuration, Frequency sample_rate)
{
	_configuration = configuration;
	_sample_rate = sample_rate;

	float const samples = 5_ms * sample_rate * _oversampling;

	// Setup smoothers:
	if (_params_1->enabled)
	{
		_smoother_1_frequency.set_samples (samples);
		_smoother_1_resonance.set_samples (samples);
		_smoother_1_gain.set_samples (samples);
		_smoother_1_attenuation.set_samples (samples);
	}

	if (_params_2->enabled)
	{
		_smoother_2_frequency.set_samples (samples);
		_smoother_2_resonance.set_samples (samples);
		_smoother_2_gain.set_samples (samples);
		_smoother_2_attenuation.set_samples (samples);
	}
}


void
DualFilter::set_oversampling (unsigned int oversampling)
{
	_oversampling = oversampling;
	configure (_configuration, _sample_rate);
}


bool
DualFilter::process (Haruhi::AudioBuffer* input_1, Haruhi::AudioBuffer* input_2,
					 Haruhi::AudioBuffer* buffer_1, Haruhi::AudioBuffer* buffer_2,
					 Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2)
{
	assert (input_1->size() == input_2->size());

	bool filtered = true;
	std::size_t nsamples = input_1->size();

	int stages1 = std::min (MaxStages, _params_1->stages.get());
	int stages2 = std::min (MaxStages, _params_2->stages.get());

	bool f1 = _params_1->enabled;
	bool f2 = _params_2->enabled;

	if (f1)
	{
		Sample const frequency = _smoother_1_frequency.process (0.5f * _params_1->frequency.get() / Params::Filter::FrequencyMax, nsamples);
		Sample const resonance = _smoother_1_resonance.process (_params_1->resonance.to_f(), nsamples);
		Sample const gain = _smoother_1_gain.process (_params_1->gain.to_f(), nsamples);
		Sample const attenuation = _smoother_1_attenuation.process (_params_1->attenuation.to_f(), nsamples);

		_impulse_response_1.set_type (static_cast<FilterImpulseResponse::Type> (_params_1->type.get()));
		_impulse_response_1.set_frequency (frequency / _oversampling);
		_impulse_response_1.set_resonance (resonance);
		_impulse_response_1.set_gain (gain);
		_impulse_response_1.set_attenuation (attenuation);
		_impulse_response_1.set_limiter_enabled (_params_1->limiter_enabled);
	}

	if (f2)
	{
		Sample const frequency = _smoother_2_frequency.process (0.5f * _params_2->frequency.get() / Params::Filter::FrequencyMax, nsamples);
		Sample const resonance = _smoother_2_resonance.process (_params_2->resonance.to_f(), nsamples);
		Sample const gain = _smoother_2_gain.process (_params_2->gain.to_f(), nsamples);
		Sample const attenuation = _smoother_2_attenuation.process (_params_2->attenuation.to_f(), nsamples);

		_impulse_response_2.set_type (static_cast<FilterImpulseResponse::Type> (_params_2->type.get()));
		_impulse_response_2.set_frequency (frequency / _oversampling);
		_impulse_response_2.set_resonance (resonance);
		_impulse_response_2.set_gain (gain);
		_impulse_response_2.set_attenuation (attenuation);
		_impulse_response_2.set_limiter_enabled (_params_2->limiter_enabled);
	}

	if (_configuration == Serial)
	{
		// Filter1 enabled only:
		if (f1 && !f2)
		{
			filterout (_filter_1[0], stages1, input_1, buffer_1, output_1);
			filterout (_filter_1[1], stages1, input_2, buffer_1, output_2);
		}
		// Filter2 enabled only:
		else if (!f1 && f2)
		{
			filterout (_filter_2[0], stages2, input_1, buffer_1, output_1);
			filterout (_filter_2[1], stages2, input_2, buffer_1, output_2);
		}
		// Both enabled:
		else if (f1 && f2)
		{
			filterout (_filter_1[0], stages1, input_1, buffer_1, buffer_2);
			filterout (_filter_2[0], stages2, buffer_2, buffer_1, output_1);
			filterout (_filter_1[1], stages1, input_2, buffer_1, buffer_2);
			filterout (_filter_2[1], stages2, buffer_2, buffer_1, output_2);
		}
		// Neither enabled:
		else
			filtered = false;
	}
	else if (_configuration == Parallel)
	{
		// Filter1 enabled only:
		if (f1 && !f2)
		{
			filterout (_filter_1[0], stages1, input_1, buffer_1, output_1);
			filterout (_filter_1[1], stages1, input_2, buffer_1, output_2);
		}
		// Filter2 enabled only:
		else if (!f1 && f2)
		{
			filterout (_filter_2[0], stages2, input_1, buffer_1, output_1);
			filterout (_filter_2[1], stages2, input_2, buffer_1, output_2);
		}
		// Both enabled:
		else if (f1 && f2)
		{
			filterout (_filter_1[0], stages1, input_1, buffer_1, output_1);
			filterout (_filter_2[0], stages2, input_1, buffer_1, buffer_2);
			// Mix in buffer into output:
			output_1->mixin (buffer_2);
			filterout (_filter_1[1], stages1, input_2, buffer_1, output_2);
			filterout (_filter_2[1], stages2, input_2, buffer_1, buffer_2);
			// Mix in buffer into output:
			output_2->mixin (buffer_2);
		}
		// Neither enabled:
		else
			filtered = false;
	}

	return filtered;
}


void
DualFilter::filterout (FilterType* filters, int stages, Haruhi::AudioBuffer* input, Haruhi::AudioBuffer* buffer, Haruhi::AudioBuffer* output) noexcept
{
	// ↓ stages
	// 1: in -> out
	// 2: in -> buf -> out
	// 3: in -> out -> buf -> out
	// 4: in -> buf -> out -> buf -> out
	// 5: in -> out -> buf -> out -> buf -> out

	Haruhi::AudioBuffer* s = buffer;
	Haruhi::AudioBuffer* t = output;
	if (stages % 2 == 0)
		std::swap (s, t);
	// Now for odd number of stages, s is buffer, t is output.
	// For even s is output, t is buffer.
	filters[0].transform (input->begin(), input->end(), t->begin());
	for (int i = 1; i < stages; ++i)
	{
		std::swap (s, t);
		filters[i].transform (s->begin(), s->end(), t->begin());
	}
}

} // namespace Yuki

