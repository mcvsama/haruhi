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

// Local:
#include "types.h"
#include "voice_oscillator.h"


namespace MikuruPrivate {

VoiceOscillator::VoiceOscillator (DSP::Wavetable* wavetable):
	_wavetable_enabled (true),
	_wavetable (wavetable),
	_frequency_source (0),
	_amplitude_source (0),
	_initial_phase_spread (0),
	// -1 to force update unison coefficients:
	_unison_number (-1),
	_unison_spread (0),
	_unison_noise (0.0),
	_unison_stereo (false),
	_noise(),
	_noise_state (_noise.state()),
	_noise_enabled (false),
	_noise_amplitude (0.0f)
{
	set_unison_number (1);
	set_phase (0);
}


void
VoiceOscillator::set_phase (Sample phase)
{
	for (int i = 0; i < _unison_number; ++i)
	{
		_phases[i] = 0.5f * (1.0f + phase);
		phase += _initial_phase_spread;
	}
}


void
VoiceOscillator::set_unison_number (int number)
{
	if (_unison_number != number)
	{
		if (number < 1)
			number = 1;
		if (number > MaxUnison)
			number = MaxUnison;

		if (number > _unison_number && _unison_number > 0)
		{
			Sample p = _phases[_unison_number-1];
			for (int i = _unison_number; i < number; ++i)
				_phases[i] = p += _initial_phase_spread;
		}

		_unison_number = number;
		update_unison_coefficients();
	}
}

} // namespace MikuruPrivate

