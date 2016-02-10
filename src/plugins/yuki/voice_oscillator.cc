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
#include "voice_oscillator.h"


namespace Yuki {

using Haruhi::Sample;


VoiceOscillator::VoiceOscillator (DSP::Wave* wave) noexcept:
	_wave_enabled (true),
	_wave (wave)
{
	set_unison_number (1);
	set_phase (0);
}


void
VoiceOscillator::set_phase (Sample phase) noexcept
{
	for (int u = 0; u < _unison_number; ++u)
	{
		_unison[u].phase = 0.5f * (1.0f + phase);
		_unison[u].vibrato_phase = 0.5f * (_noise.get (_noise_state) + 1.0f);
		phase += _initial_phase_spread;
	}
}


void
VoiceOscillator::set_unison_number (int number) noexcept
{
	if (_unison_number != number)
	{
		if (number < 1)
			number = 1;
		if (number > MaxUnison)
			number = MaxUnison;

		if (number > _unison_number && _unison_number > 0)
		{
			Sample p = _unison[_unison_number-1].phase;
			for (int u = _unison_number; u < number; ++u)
			{
				_unison[u].phase = p += _initial_phase_spread;
				_unison[u].vibrato_phase = 0.5f * (_noise.get (_noise_state) + 1.0f);
			}
		}

		_unison_number = number;
		update_unison_coefficients();
	}
}

} // namespace Yuki

