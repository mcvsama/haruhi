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
	OnePoleSmoother (Sample speed = 0.005f)
	{
		set_speed (speed);
		reset();
	}

	void
	set_speed (Sample speed)
	{
		_a = 1.f - speed;
		_b = speed;
	}

	/**
	 * Resets smoother to initial state (value of 0.0).
	 */
	void
	reset()
	{
		_z = 0.0f;
	}

	/**
	 * Return smoothed sample from given input sample.
	 */
	Sample
	process (Sample s, unsigned int iterations = 1)
	{
		for (unsigned int i = 0; i < iterations; ++i)
			_z = (s * _b) + (_z * _a);
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
	Sample _a, _b, _z;
};

} // namespace DSP

} // namespace Haruhi

#endif

