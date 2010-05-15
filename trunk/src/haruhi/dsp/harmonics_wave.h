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

#ifndef HARUHI__DSP__HARMONICS_WAVE_H__INCLUDED
#define HARUHI__DSP__HARMONICS_WAVE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Haruhi:
#include <haruhi/core/audio.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/utility/numeric.h>


namespace DSP {

/**
 * Decorator for Wave that adds harmonics to it.
 */
class HarmonicsWave: public Wave
{
  public:
	enum {
		HarmonicsNumber = 24,
	};

	struct Harmonic
	{
		Harmonic():
			value (0),
			phase (0)
		{ }

		Core::Sample	value;
		Core::Sample	phase;
	};

	typedef std::vector<Harmonic> Harmonics;

  public:
	HarmonicsWave (Wave* wave = 0, bool auto_delete = false);

	~HarmonicsWave();

	/**
	 * Returns sample from base function with added harmonics.
	 * \param	phase is the phase in range [0, 1].
	 * \param	frequency is the base frequency of the signal this sample will
	 * 			be used in (this is for limiting bandwidth).
	 */
	Core::Sample
	operator() (Core::Sample register phase, Core::Sample frequency) const
	{
		Core::Sample sum = 0;
		for (Harmonics::size_type h = 0, n = _harmonics.size(); h < n; ++h)
			sum += _harmonics[h].value * _wave->operator() (mod1 (0.5f * _harmonics[h].phase + phase * (h + 1)), frequency);
		return sum;
	}

	/**
	 * Returns wave object.
	 */
	Wave*
	wave() const { return _wave; }

	/**
	 * Sets new wave to be decorated. Drops old wave if auto_delete has been set.
	 */
	void
	set_wave (Wave* wave);

	/**
	 * Tells whether to delete decorated wave upon destruction.
	 * (If wave is 0, it's not deleted.)
	 */
	void
	set_auto_delete (bool set) { _auto_delete = set; }

	/**
	 * Sets given harmonic.
	 * \param	index is harmonic number, must be less that HarmonicNumber.
	 */
	void
	set_harmonic (int index, Harmonic const& harmonic);

	void
	set_harmonic (int index, Core::Sample value, Core::Sample phase);

	/**
	 * Gets given harmonic.
	 * \param	index is harmonic number, must be less that HarmonicNumber.
	 */
	Harmonic const&
	harmonic (int index) const { return _harmonics[index]; }

	/**
	 * Returns harmonics vector.
	 */
	Harmonics&
	harmonics() { return _harmonics; }

	Harmonics const&
	harmonics() const { return _harmonics; }

  private:
	Wave*		_wave;
	Harmonics	_harmonics;
	bool		_auto_delete;
};

} // namespace DSP

#endif

