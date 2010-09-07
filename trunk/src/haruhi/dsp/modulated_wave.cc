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
#include "modulated_wave.h"


namespace DSP {

ModulatedWave::ModulatedWave (Wave* wave, Wave* modulator, Type mod_type, Core::Sample mod_amplitude, unsigned int mod_index, bool auto_delete_wave, bool auto_delete_modulator):
	Wave (wave ? wave->immutable() : false),
	_wave (wave),
	_modulator (modulator),
	_mod_type (mod_type),
	_mod_amplitude (mod_amplitude),
	_mod_index (mod_index),
	_auto_delete_wave (auto_delete_wave),
	_auto_delete_modulator (auto_delete_modulator)
{
	assert (mod_index >= 1);

	switch (_mod_type)
	{
		case Ring:		_value_function = &ModulatedWave::value_for_ring;		break;
		case Frequency:	_value_function = &ModulatedWave::value_for_frequency;	break;
	}
}


ModulatedWave::~ModulatedWave()
{
	if (_auto_delete_wave)
		delete _wave;
	if (_auto_delete_modulator)
		delete _modulator;
}


Core::Sample
ModulatedWave::operator() (Core::Sample phase, Core::Sample frequency) const
{
	return (this->*_value_function) (phase, frequency);	
}


void
ModulatedWave::set_wave (Wave* wave)
{
	if (_auto_delete_wave)
		delete _wave;
	_wave = wave;
	set_immutable (_wave ? _wave->immutable() : false);
}


void
ModulatedWave::set_modulator (Wave* modulator)
{
	if (_auto_delete_modulator)
		delete _modulator;
	_modulator = modulator;
}


Core::Sample
ModulatedWave::value_for_ring (Core::Sample phase, Core::Sample frequency) const
{
	return (*_wave)(phase, frequency) * (1.0f - _mod_amplitude * (*_modulator)(mod1 (phase * _mod_index), frequency * _mod_index));
}


Core::Sample
ModulatedWave::value_for_frequency (Core::Sample phase, Core::Sample frequency) const
{
	return (*_wave)(mod1 (phase * (1.0f - 0.1 * _mod_amplitude * (*_modulator)(mod1 (phase * _mod_index), frequency * _mod_index))), frequency);
}

} // namespace DSP

