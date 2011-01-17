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

// Local:
#include "mikuru.h"
#include "double_filter.h"


namespace MikuruPrivate {

DoubleFilter::DoubleFilter (Mikuru* mikuru):
	_mikuru (mikuru),
	_impulse_response1 (RBJImpulseResponse::LowPass, 0.0, 0.0, 0.0, 1.0),
	_impulse_response2 (RBJImpulseResponse::LowPass, 0.0, 0.0, 0.0, 1.0)
{
	for (int i = 0; i < NumFilters; ++i)
	{
		_filter1[i].assign_impulse_response (&_impulse_response1);
		_filter2[i].assign_impulse_response (&_impulse_response2);
	}
}


void
DoubleFilter::reset()
{
	for (int i = 0; i < NumFilters; ++i)
	{
		_filter1[i].reset();
		_filter2[i].reset();
	}
}


void
DoubleFilter::configure (Configuration configuration, Params::Filter* params1, Params::Filter* params2)
{
	_configuration = configuration;
	_params1 = *params1;
	_params2 = *params2;

	float const samples = 0.005f * _mikuru->graph()->sample_rate();

	// Setup smoothers:
	if (_params1.enabled)
	{
		_smoother_filter1_frequency.set_samples (samples);
		_smoother_filter1_resonance.set_samples (samples);
		_smoother_filter1_gain.set_samples (samples);
		_smoother_filter1_attenuation.set_samples (samples);
	}

	if (_params2.enabled)
	{
		_smoother_filter2_frequency.set_samples (samples);
		_smoother_filter2_resonance.set_samples (samples);
		_smoother_filter2_gain.set_samples (samples);
		_smoother_filter2_attenuation.set_samples (samples);
	}
}


bool
DoubleFilter::process (Haruhi::AudioBuffer& input, Haruhi::AudioBuffer& buffer1, Haruhi::AudioBuffer& buffer2, Haruhi::AudioBuffer& output)
{
	bool filtered = true;
	std::size_t nsamples = std::distance (input.begin(), input.end());

	int passes1 = std::min (static_cast<int> (NumFilters), _params1.passes.get());
	int passes2 = std::min (static_cast<int> (NumFilters), _params2.passes.get());

	bool f1 = _params1.enabled;
	bool f2 = _params2.enabled;

	if (f1)
	{
		Sample const frequency = _smoother_filter1_frequency.process (0.5f * _params1.frequency.get() / Params::Filter::FrequencyMax, nsamples);
		Sample const resonance = _smoother_filter1_resonance.process (_params1.resonance.to_f(), nsamples);
		Sample const gain = _smoother_filter1_gain.process (_params1.gain.to_f(), nsamples);
		Sample const attenuation = _smoother_filter1_attenuation.process (_params1.attenuation.to_f(), nsamples);

		_impulse_response1.set_type (static_cast<RBJImpulseResponse::Type> (_params1.type.get()));
		_impulse_response1.set_frequency (frequency);
		_impulse_response1.set_resonance (resonance);
		_impulse_response1.set_gain (gain);
		_impulse_response1.set_attenuation (attenuation);
		_impulse_response1.set_limiter (_params1.limiter_enabled);
	}

	if (f2)
	{
		Sample const frequency = _smoother_filter1_frequency.process (0.5f * _params2.frequency.get() / Params::Filter::FrequencyMax, nsamples);
		Sample const resonance = _smoother_filter1_resonance.process (_params2.resonance.to_f(), nsamples);
		Sample const gain = _smoother_filter1_gain.process (_params2.gain.to_f(), nsamples);
		Sample const attenuation = _smoother_filter1_attenuation.process (_params2.attenuation.to_f(), nsamples);

		_impulse_response2.set_type (static_cast<RBJImpulseResponse::Type> (_params2.type.get()));
		_impulse_response2.set_frequency (frequency);
		_impulse_response2.set_resonance (resonance);
		_impulse_response2.set_gain (gain);
		_impulse_response2.set_attenuation (attenuation);
		_impulse_response2.set_limiter (_params2.limiter_enabled);
	}

	if (_configuration == ConfigurationSerial)
	{
		// Filter1 enabled only:
		if (f1 && !f2)
			filterout (_filter1, passes1, input, buffer1, output);
		// Filter2 enabled only:
		else if (!f1 && f2)
			filterout (_filter2, passes2, input, buffer1, output);
		// Both enabled:
		else if (f1 && f2)
		{
			filterout (_filter1, passes1, input, buffer1, buffer2);
			filterout (_filter2, passes2, buffer2, buffer1, output);
		}
		// Neither enabled:
		else
			filtered = false;
	}
	else if (_configuration == ConfigurationParallel)
	{
		// Filter1 enabled only:
		if (f1 && !f2)
			filterout (_filter1, passes1, input, buffer1, output);
		// Filter2 enabled only:
		else if (!f1 && f2)
			filterout (_filter2, passes2, input, buffer1, output);
		// Both enabled:
		else if (f1 && f2)
		{
			filterout (_filter1, passes1, input, buffer1, output);
			filterout (_filter2, passes2, input, buffer1, buffer2);
			// Mix in buffer into output:
			output.mixin (&buffer2);
		}
		// Neither enabled:
		else
			filtered = false;
	}

	return filtered;
}


void
DoubleFilter::filterout (DSP::Filter* filters, int passes, Haruhi::AudioBuffer& input, Haruhi::AudioBuffer& buffer, Haruhi::AudioBuffer& output)
{
	// ↓ passes
	// 1: in -> out
	// 2: in -> buf -> out
	// 3: in -> out -> buf -> out
	// 4: in -> buf -> out -> buf -> out
	// 5: in -> out -> buf -> out -> buf -> out

	Haruhi::AudioBuffer* s = &buffer;
	Haruhi::AudioBuffer* t = &output;
	if (passes % 2 == 0)
		std::swap (s, t);
	// Now for odd number of passes, s is buffer, t is output.
	// For even s is output, t is buffer.
	filters[0].transform (input.begin(), input.end(), t->begin());
	for (int i = 1; i < passes; ++i)
	{
		std::swap (s, t);
		filters[i].transform (s->begin(), s->end(), t->begin());
	}
}

} // namespace MikuruPrivate

