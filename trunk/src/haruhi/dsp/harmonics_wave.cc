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

// Haruhi:
#include <haruhi/config/system.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "harmonics_wave.h"


namespace DSP {

HarmonicsWave::HarmonicsWave (Wave* wave, bool auto_delete):
	_wave (wave),
	_auto_delete (auto_delete)
{
	_harmonics.resize (HarmonicsNumber);
}


HarmonicsWave::~HarmonicsWave()
{
	if (_auto_delete)
		delete _wave;
}


void
HarmonicsWave::set_wave (Wave* wave)
{
	if (_auto_delete)
		delete _wave;
	_wave = wave;
}


void
HarmonicsWave::set_harmonic (unsigned int index, Harmonic const& harmonic)
{
	assert (index < HarmonicsNumber);

	_harmonics[index] = harmonic;
}


void
HarmonicsWave::set_harmonic (unsigned int index, Core::Sample value, Core::Sample phase)
{
	assert (index < HarmonicsNumber);

	_harmonics[index].value = value;
	_harmonics[index].phase = phase;
}

} // namespace DSP

