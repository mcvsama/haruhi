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

#ifndef HARUHI__DSP__SCALED_WAVE_H__INCLUDED
#define HARUHI__DSP__SCALED_WAVE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>


namespace Haruhi {

namespace DSP {

/**
 * A Wave that is scaled by some fixed factor.
 */
class ScaledWave: public Wave
{
  public:
	/**
	 * \param	auto_delete Should inner wave be deleted when this one is?
	 */
	ScaledWave (Sample scale, Wave* wave, bool auto_delete = false) noexcept;

	Sample
	operator() (Sample register phase, Sample frequency) const noexcept;

  private:
	Sample _scale;
};


inline
ScaledWave::ScaledWave (Sample scale, Wave* wave, bool auto_delete) noexcept:
	Wave (wave, auto_delete),
	_scale (scale)
{ }


inline Sample
ScaledWave::operator() (Sample phase, Sample frequency) const noexcept
{
	return _scale * (*inner_wave())(phase, frequency);
}

} // namespace DSP

} // namespace Haruhi

#endif

