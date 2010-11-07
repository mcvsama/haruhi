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
#include "oscillator.h"


namespace Haruhi {

namespace DSP {

using Core::Sample;


Oscillator::Oscillator (Wavetable* wavetable):
	_wavetable (wavetable),
	_frequency (0),
	_value (1.0),
	_initial_phase_spread (0),
	_unison_number (-1),
	_unison_spread (0),
	_unison_noise (0.0),
	_frequency_modulator (0),
	_noise(),
	_noise_state (_noise.state())
{
	reset();
	set_unison_number (1);
}


void
Oscillator::set_phase (Sample phase)
{
	assert (_unison_number >= 1);

	for (unsigned int i = 0; i < _unison_number; ++i)
	{
		_phases[i] = phase;
		phase += _initial_phase_spread;
	}
}


void
Oscillator::set_unison_number (unsigned int number)
{
	assert (number >= 1);
	assert (number < MaxUnison);

	if (_unison_number != number)
	{
		if (number > _unison_number && _unison_number > 0)
		{
			Sample p = _phases[_unison_number-1];
			for (unsigned int i = _unison_number; i < number; ++i)
				_phases[i] = p += _initial_phase_spread;
		}

		_unison_number = number;
		update_unison_coefficients();
	}
}


void
Oscillator::reset()
{
	set_phase (0);
}

} // namespace DSP

} // namespace Haruhi

