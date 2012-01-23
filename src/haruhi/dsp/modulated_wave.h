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

#ifndef HARUHI__DSP__MODULATED_WAVE_H__INCLUDED
#define HARUHI__DSP__MODULATED_WAVE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>


namespace Haruhi {

namespace DSP {

/**
 * Decorator for Wave that modulates it.
 * Available modulations are: amplitude modulation, frequency modulation, ring modulation.
 */
class ModulatedWave: public Wave
{
	typedef Sample (ModulatedWave::*ValueFunction)(Sample, Sample) const;

  public:
	enum Type {
		Ring		= 0,
		Frequency	= 1,
	};

  public:
	/**
	 * \param	mod_index indicates number of times modulator frequency is greater than wave's.
	 */
	ModulatedWave (Wave* inner_wave = 0, Wave* modulator = 0, Type mod_type = Ring, float mod_amplitude = 0.0f, unsigned int mod_index = 1, bool auto_delete_wave = false, bool auto_delete_modulator = false) noexcept;

	/**
	 * Dtor
	 */
	~ModulatedWave() noexcept;

	/**
	 * Returns modulator wave object.
	 */
	Wave*
	modulator() const noexcept;

	/**
	 * Sets new modulator wave. Drops old modulator if auto_delete has been set.
	 */
	void
	set_modulator (Wave* modulator, bool auto_delete) noexcept;

	/**
	 * Returns sample from base function modulated with modulator.
	 * \param	phase is the phase in range [-1, 1].
	 * \param	frequency is the base frequency of the signal this sample will
	 * 			be used in (this is for limiting bandwidth).
	 */
	Sample
	operator() (Sample register phase, Sample frequency) const noexcept;

  private:
	Sample
	value_for_ring (Sample phase, Sample frequency) const noexcept;

	Sample
	value_for_frequency (Sample phase, Sample frequency) const noexcept;

  private:
	Wave*			_modulator;
	Type			_mod_type;
	Sample			_mod_amplitude;
	unsigned int	_mod_index;
	bool			_auto_delete_modulator;
	ValueFunction	_value_function;
};


inline
ModulatedWave::ModulatedWave (Wave* inner_wave, Wave* modulator, Type mod_type, Sample mod_amplitude, unsigned int mod_index, bool auto_delete_wave, bool auto_delete_modulator) noexcept:
	Wave (inner_wave, auto_delete_wave),
	_modulator (modulator),
	_mod_type (mod_type),
	_mod_amplitude (mod_amplitude),
	_mod_index (mod_index),
	_auto_delete_modulator (auto_delete_modulator)
{
	assert (mod_index >= 1);

	switch (_mod_type)
	{
		case Ring:		_value_function = &ModulatedWave::value_for_ring;		break;
		case Frequency:	_value_function = &ModulatedWave::value_for_frequency;	break;
	}
}


inline
ModulatedWave::~ModulatedWave() noexcept
{
	if (_auto_delete_modulator)
		delete _modulator;
}


inline Wave*
ModulatedWave::modulator() const noexcept
{
	return _modulator;
}


inline void
ModulatedWave::set_modulator (Wave* modulator, bool auto_delete) noexcept
{
	if (_auto_delete_modulator)
		delete _modulator;
	_auto_delete_modulator = auto_delete;
	_modulator = modulator;
}


inline Sample
ModulatedWave::value_for_ring (Sample phase, Sample frequency) const noexcept
{
	Sample const x = (*inner_wave()) (phase, frequency);
	Sample const m = (*_modulator) (mod1 (phase * _mod_index), frequency * _mod_index);
	Sample const& a = _mod_amplitude;
	return x * (1.0f - a + a * m); // Simplified: (1-a)x + a(xm)
}


inline Sample
ModulatedWave::value_for_frequency (Sample phase, Sample frequency) const noexcept
{
	Sample const m = (*_modulator)(mod1 (phase * _mod_index), frequency * _mod_index);
	return (*inner_wave())(mod1 (phase + phase * _mod_amplitude * m), frequency);
}


inline Sample
ModulatedWave::operator() (Sample phase, Sample frequency) const noexcept
{
	return (this->*_value_function) (phase, frequency);
}

} // namespace DSP

} // namespace Haruhi

#endif

