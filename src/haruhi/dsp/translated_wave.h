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

#ifndef HARUHI__DSP__TRANSLATED_WAVE_H__INCLUDED
#define HARUHI__DSP__TRANSLATED_WAVE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>


namespace Haruhi {

namespace DSP {

/**
 * A Wave that has values corrected by some fixed delta.
 */
class TranslatedWave: public Wave
{
  public:
	/**
	 * \param	auto_delete Should inner wave be deleted when this one is?
	 */
	TranslatedWave (Sample delta, Wave* wave, bool auto_delete = false);

	Sample
	operator() (Sample register phase, Sample frequency) const;

  private:
	Sample _delta;
};


inline
TranslatedWave::TranslatedWave (Sample delta, Wave* wave, bool auto_delete):
	Wave (wave, auto_delete),
	_delta (delta)
{ }


inline Sample
TranslatedWave::operator() (Sample phase, Sample frequency) const
{
	return _delta + (*inner_wave())(phase, frequency);
}

} // namespace DSP

} // namespace Haruhi

#endif

