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

#ifndef HARUHI__DSP__MODULATED_WAVE_H__INCLUDED
#define HARUHI__DSP__MODULATED_WAVE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Haruhi:
#include <haruhi/core/audio.h>
#include <haruhi/dsp/wave.h>


namespace DSP {

/**
 * Decorator for Wave that modulates it.
 * Available modulations are: amplitude modulation, frequency modulation, ring modulation.
 */
class ModulatedWave: public Wave
{
	typedef Core::Sample (ModulatedWave::*ValueFunction)(Core::Sample, Core::Sample) const;

  public:
	enum Type {
		Ring		= 0,
		Frequency	= 1,
	};

  public:
	/**
	 * \param	mod_index indicates number of times modulator frequency is greater than wave's.
	 */
	ModulatedWave (Wave* wave = 0, Wave* modulator = 0, Type mod_type = Ring, float mod_amplitude = 0.0f, int mod_index = 1, bool auto_delete_wave = false, bool auto_delete_modulator = false);

	~ModulatedWave();

	/**
	 * Returns sample from base function modulated with modulator.
	 * \param	phase is the phase in range [-1, 1].
	 * \param	frequency is the base frequency of the signal this sample will
	 * 			be used in (this is for limiting bandwidth).
	 */
	Core::Sample
	operator() (Core::Sample register phase, Core::Sample frequency) const;

	/**
	 * Returns wave object.
	 */
	Wave*
	wave() const { return _wave; }

	/**
	 * Returns modulator wave object.
	 */
	Wave*
	modulator() const { return _modulator; }

	/**
	 * Sets new wave to be decorated. Drops old wave if auto_delete has been set.
	 */
	void
	set_wave (Wave* wave);

	/**
	 * Sets new modulator wave. Drops old modulator if auto_delete has been set.
	 */
	void
	set_modulator (Wave* modulator);

	/**
	 * Tells whether to delete decorated wave upon destruction.
	 * (If wave is 0, it's not deleted.)
	 */
	void
	set_auto_delete_wave (bool set) { _auto_delete_wave = set; }

	/**
	 * Tells whether to delete used modulating wave upon destruction.
	 * (If modulator is 0, it's not deleted.)
	 */
	void
	set_auto_delete_modulator (bool set) { _auto_delete_modulator = set; }

  private:
	Core::Sample
	value_for_ring (Core::Sample phase, Core::Sample frequency) const;

	Core::Sample
	value_for_frequency (Core::Sample phase, Core::Sample frequency) const;

  private:
	Wave*			_wave;
	Wave*			_modulator;
	Type			_mod_type;
	Core::Sample	_mod_amplitude;
	int				_mod_index;
	bool			_auto_delete_wave;
	bool			_auto_delete_modulator;
	ValueFunction	_value_function;
};

} // namespace DSP

#endif

