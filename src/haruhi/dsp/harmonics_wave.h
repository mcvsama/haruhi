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

#ifndef HARUHI__DSP__HARMONICS_WAVE_H__INCLUDED
#define HARUHI__DSP__HARMONICS_WAVE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/utility/numeric.h>


namespace Haruhi {

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
		Harmonic() noexcept;

		Sample	value;
		Sample	phase;
	};

	typedef std::vector<Harmonic> Harmonics;

  public:
	/**
	 * Create wave that adds harmonics to inner wave.
	 */
	HarmonicsWave (Wave* inner_wave = nullptr, bool auto_delete = false) noexcept;

	/**
	 * Returns sample from base function with added harmonics.
	 * \param	phase is the phase in range [0, 1].
	 * \param	frequency is the base frequency of the signal this sample will
	 *			be used in (this is for limiting bandwidth).
	 */
	Sample
	operator() (Sample phase, Sample frequency, std::size_t) const noexcept override;

	/**
	 * Gets given harmonic.
	 * \param	index is harmonic number, must be less that HarmonicNumber.
	 */
	Harmonic const&
	harmonic (int index) const noexcept;

	/**
	 * Sets given harmonic.
	 * \param	index is harmonic number, must be less that HarmonicNumber.
	 */
	void
	set_harmonic (unsigned int index, Harmonic const& harmonic) noexcept;

	void
	set_harmonic (unsigned int index, Sample value, Sample phase) noexcept;

	/**
	 * Returns harmonics vector.
	 */
	Harmonics&
	harmonics() noexcept;

	/**
	 * Returns harmonics vector.
	 */
	Harmonics const&
	harmonics() const noexcept;

  private:
	Harmonics _harmonics;
};


inline
HarmonicsWave::Harmonic::Harmonic() noexcept:
	value (0.0f),
	phase (0.0f)
{ }


inline
HarmonicsWave::HarmonicsWave (Wave* wave, bool auto_delete) noexcept:
	Wave (wave, auto_delete)
{
	_harmonics.resize (HarmonicsNumber);
}


inline Sample
HarmonicsWave::operator() (Sample phase, Sample frequency, std::size_t sample) const noexcept
{
	Sample sum = 0;
	for (Harmonics::size_type h = 0, n = _harmonics.size(); h < n; ++h)
		sum += _harmonics[h].value * (*inner_wave())(mod1 (0.5f * _harmonics[h].phase + phase * (h + 1)), frequency, sample);
	return sum;
}


inline HarmonicsWave::Harmonic const&
HarmonicsWave::harmonic (int index) const noexcept
{
	return _harmonics[index];
}


inline void
HarmonicsWave::set_harmonic (unsigned int index, Harmonic const& harmonic) noexcept
{
	assert (index < HarmonicsNumber);

	_harmonics[index] = harmonic;
}


inline void
HarmonicsWave::set_harmonic (unsigned int index, Sample value, Sample phase) noexcept
{
	assert (index < HarmonicsNumber);

	_harmonics[index].value = value;
	_harmonics[index].phase = phase;
}


inline HarmonicsWave::Harmonics&
HarmonicsWave::harmonics() noexcept
{
	return _harmonics;
}


inline HarmonicsWave::Harmonics const&
HarmonicsWave::harmonics() const noexcept
{
	return _harmonics;
}

} // namespace DSP

} // namespace Haruhi

#endif

