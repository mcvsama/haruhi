/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
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

	int sr = _mikuru->graph()->sample_rate();

	// Setup smoothers:
	_smoother_filter1_frequency.set_smoothing_samples (0.001f * atomic (_params1.frequency_smoothing) * sr);
	_smoother_filter1_resonance.set_smoothing_samples (0.001f * atomic (_params1.resonance_smoothing) * sr);
	_smoother_filter1_gain.set_smoothing_samples (0.001f * atomic (_params1.gain_smoothing) * sr);
	_smoother_filter1_attenuation.set_smoothing_samples (0.001f * atomic (_params1.attenuation_smoothing) * sr);

	_smoother_filter2_frequency.set_smoothing_samples (0.001f * atomic (_params2.frequency_smoothing) * sr);
	_smoother_filter2_resonance.set_smoothing_samples (0.001f * atomic (_params2.resonance_smoothing) * sr);
	_smoother_filter2_gain.set_smoothing_samples (0.001f * atomic (_params2.gain_smoothing) * sr);
	_smoother_filter2_attenuation.set_smoothing_samples (0.001f * atomic (_params2.attenuation_smoothing) * sr);

	if (_params1.enabled)
	{
		_smoother_filter1_frequency.set_value (0.5f * _params1.frequency / Params::Filter::FrequencyMax);
		_smoother_filter1_resonance.set_value (1.0f * _params1.resonance / Params::Filter::ResonanceDenominator);
		_smoother_filter1_gain.set_value (1.0f * _params1.gain / Params::Filter::GainDenominator);
		_smoother_filter1_attenuation.set_value (1.0f * _params1.attenuation / Params::Filter::AttenuationDenominator);
	}

	if (_params2.enabled)
	{
		_smoother_filter2_frequency.set_value (0.5f * _params2.frequency / Params::Filter::FrequencyMax);
		_smoother_filter2_resonance.set_value (1.0f * _params2.resonance / Params::Filter::ResonanceDenominator);
		_smoother_filter2_gain.set_value (1.0f * _params2.gain / Params::Filter::GainDenominator);
		_smoother_filter2_attenuation.set_value (1.0f * _params2.attenuation / Params::Filter::AttenuationDenominator);
	}
}


bool
DoubleFilter::process (Core::AudioBuffer& input, Core::AudioBuffer& buffer1, Core::AudioBuffer& buffer2, Core::AudioBuffer& output)
{
	bool filtered = true;
	std::size_t nsamples = std::distance (input.begin(), input.end());

	bool f1 = _params1.enabled;
	bool f2 = _params2.enabled;

	int passes1 = std::min (static_cast<int> (NumFilters), _params1.passes);
	int passes2 = std::min (static_cast<int> (NumFilters), _params2.passes);

	if (f1)
	{
		_smoother_filter1_frequency.advance (nsamples);
		_smoother_filter1_resonance.advance (nsamples);
		_smoother_filter1_gain.advance (nsamples);
		_smoother_filter1_attenuation.advance (nsamples);

		_impulse_response1.set_type (static_cast<RBJImpulseResponse::Type> (_params1.type));
		_impulse_response1.set_frequency (_smoother_filter1_frequency.current());
		_impulse_response1.set_resonance (_smoother_filter1_resonance.current());
		_impulse_response1.set_gain (_smoother_filter1_gain.current());
		_impulse_response1.set_attenuation (_smoother_filter1_attenuation.current());
		_impulse_response1.set_limiter (_params1.limiter_enabled);
	}

	if (f2)
	{
		_smoother_filter2_frequency.advance (nsamples);
		_smoother_filter2_resonance.advance (nsamples);
		_smoother_filter2_gain.advance (nsamples);
		_smoother_filter2_attenuation.advance (nsamples);

		_impulse_response2.set_type (static_cast<RBJImpulseResponse::Type> (_params2.type));
		_impulse_response2.set_frequency (_smoother_filter2_frequency.current());
		_impulse_response2.set_resonance (_smoother_filter2_resonance.current());
		_impulse_response2.set_gain (_smoother_filter2_gain.current());
		_impulse_response2.set_attenuation (_smoother_filter2_attenuation.current());
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
DoubleFilter::filterout (DSP::Filter* filters, int passes, Core::AudioBuffer& input, Core::AudioBuffer& buffer, Core::AudioBuffer& output)
{
	// ↓ passes
	// 1: in -> out
	// 2: in -> buf -> out
	// 3: in -> out -> buf -> out
	// 4: in -> buf -> out -> buf -> out
	// 5: in -> out -> buf -> out -> buf -> out

	Core::AudioBuffer* s = &buffer;
	Core::AudioBuffer* t = &output;
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

