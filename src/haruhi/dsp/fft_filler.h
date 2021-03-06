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

#ifndef HARUHI__DSP__FFT_FILLER_H__INCLUDED
#define HARUHI__DSP__FFT_FILLER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>
#include <functional>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/dsp/wavetable.h>


namespace Haruhi {

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
	 * \param	scale_epsilon Don't autoscale if max value is smaller than this.
	 */
	FFTFiller (Wave* wave, bool autoscale, Sample scale_epsilon = 0.0f) noexcept;

	/**
	 * Set cancel predicate.
	 *
	 * If cancel predicate returns true, the filling is interrupted and
	 * fill() method returns false. Cancel predicate should be as fast as possible,
	 * since it will be called many times during the fill.
	 */
	void
	set_cancel_predicate (std::function<bool()> cancel_predicate) noexcept;

	/**
	 * Fill the wavetable.
	 */
	void
	fill (Wavetable* wavetable, unsigned int samples);

	/**
	 * Return the wave used to fill wavetables.
	 */
	Wave*
	wave() const noexcept;

	/**
	 * Return true if last fill operation was interrupted by cancel predicate.
	 */
	bool
	was_interrupted() const noexcept;

  private:
	/**
	 * Return true if cancel predicate is set and returns true.
	 * Also set _was_interrupted flag.
	 */
	bool
	interrupted() noexcept;

  private:
	Wave*					_wave;
	bool					_autoscale;
	Sample					_scale_epsilon;
	std::function<bool()>	_cancel_predicate;
	bool					_was_interrupted;
};


inline void
FFTFiller::set_cancel_predicate (std::function<bool()> cancel_predicate) noexcept
{
	_cancel_predicate = cancel_predicate;
}


inline Wave*
FFTFiller::wave() const noexcept
{
	return _wave;
}


inline bool
FFTFiller::was_interrupted() const noexcept
{
	return _was_interrupted;
}


inline bool
FFTFiller::interrupted() noexcept
{
	return _was_interrupted = _cancel_predicate && _cancel_predicate();
}

} // namespace DSP

} // namespace Haruhi

#endif

