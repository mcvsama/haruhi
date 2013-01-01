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

// Local:
#include "voice_operator.h"


namespace Yuki {

void
VoiceOperator::fill (Haruhi::AudioBuffer* output, Haruhi::AudioBuffer* fm_output) noexcept
{
	assert (output != nullptr);
	assert (fm_output != nullptr);
	assert (output->size() == fm_output->size());

	Sample* const fs = _frequency_source->begin();
	Sample f;
	Sample p = _phase;

	// Oscillate:
	for (std::size_t i = 0; i < output->size(); ++i)
	{
		f = bound (fs[i] * _detune, 0.0f, 0.5f);
		p = mod1 (p + f);
		(*fm_output)[i] = Haruhi::DSP::base_sin<5, Haruhi::Sample> (p * 2.0f - 1.0f);
	}

	_phase = p;

	// Amplitude modulation:
	output->fill (fm_output);
	output->attenuate (_amplitude_source);
}

} // namespace Yuki

