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

#ifndef HARUHI__DSP__FFT_FILLER_H__INCLUDED
#define HARUHI__DSP__FFT_FILLER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>

// Haruhi:
#include <haruhi/core/audio.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/dsp/wavetable.h>


namespace DSP {

/**
 * Uses FFT to fill Wavetables using given Wave object.
 */
class FFTFiller: public Wavetable::Filler
{
  public:
	/**
	 * Creates filler.
	 * \param	wave is a wave object to use.
	 * \param	autoscale tells whether to force values to be within [-1..1].
	 */
	FFTFiller (Wave* wave, bool autoscale);

	void
	fill (Wavetable* wavetable, unsigned int samples);

	Wave*
	wave() const { return _wave; }

  private:
	Wave*	_wave;
	bool	_autoscale;
};

} // namespace DSP

#endif

