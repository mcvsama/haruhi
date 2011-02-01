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

#ifndef HARUHI__DSP__ONE_POLE_SMOOTHER_H__INCLUDED
#define HARUHI__DSP__ONE_POLE_SMOOTHER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Haruhi:
#include <haruhi/config/types.h>


namespace Haruhi {

namespace DSP {

/**
 * Implementation of one-pole LPF.
 * See http://musicdsp.org/showone.php?id=257
 */
class OnePoleSmoother
{
  public:
	OnePoleSmoother (Sample samples = 1.f)
	{
		set_samples (samples);
		reset();
	}

	/**
	 * \param	samples is number of samples after which returned value reaches
	 * 			99.99% of target value.
	 */
	void
	set_samples (Sample samples)
	{
		_time = std::pow (0.01f, 2.0f / samples);
	}

	/**
	 * Resets smoother to initial state (or given value).
	 */
	void
	reset (float value = 0.0f)
	{
		_z = value;
	}

	/**
	 * Return smoothed sample from given input sample.
	 */
	Sample
	process (Sample s, unsigned int iterations = 1)
	{
		for (unsigned int i = 0; i < iterations; ++i)
			_z = _time * (_z - s) + s;
		return _z;
	}

	/**
	 * Smooth (low-pass) given sequence.
	 */
	template<class ForwardIterator>
		void
		process (ForwardIterator begin, ForwardIterator end)
		{
			for (ForwardIterator c = begin; c != end; ++c)
				*c = process_sample (*c);
		}

	/**
	 * Fill sequence with smoothed samples, where input sample is @value.
	 * \param	begin,end Sequence to be filled.
	 * \param	value Input data that is to be smoothed. It's used for each
	 * 			sample in [begin, end) sequence.
	 */
	template<class ForwardIterator>
		void
		fill (ForwardIterator begin, ForwardIterator end, Sample value)
		{
			for (ForwardIterator c = begin; c != end; ++c)
				*c = process (value);
		}

	/**
	 * Multiply sequence samples by given value in a smooth-way.
	 * \param	begin,end Sequence to modify.
	 * \param	value Multiplying value.
	 */
	template<class ForwardIterator>
		void
		multiply (ForwardIterator begin, ForwardIterator end, Sample value)
		{
			for (ForwardIterator c = begin; c != end; ++c)
				*c *= process (value);
		}

  private:
	Sample _time;
	Sample _z;
};

} // namespace DSP

} // namespace Haruhi

#endif
