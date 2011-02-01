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

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "harmonics_wave.h"


namespace Haruhi {

namespace DSP {

HarmonicsWave::HarmonicsWave (Wave* wave, bool auto_delete):
	Wave (wave ? wave->immutable() : false),
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
	set_immutable (_wave ? _wave->immutable() : false);
}


void
HarmonicsWave::set_harmonic (unsigned int index, Harmonic const& harmonic)
{
	assert (index < HarmonicsNumber);

	_harmonics[index] = harmonic;
}


void
HarmonicsWave::set_harmonic (unsigned int index, Sample value, Sample phase)
{
	assert (index < HarmonicsNumber);

	_harmonics[index].value = value;
	_harmonics[index].phase = phase;
}

} // namespace DSP

} // namespace Haruhi
