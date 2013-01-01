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

#ifndef HARUHI__DSP__ONE_POLE_SMOOTHER_H__INCLUDED
#define HARUHI__DSP__ONE_POLE_SMOOTHER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/types.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/fast_pow.h>


namespace Haruhi {

namespace DSP {

/**
 * Implementation of one-pole LPF.
 * See http://musicdsp.org/showone.php?id=257
 */
class OnePoleSmoother
{
  public:
	OnePoleSmoother (Sample samples = 1.f) noexcept;

	/**
	 * \param	samples is number of samples after which returned value reaches
	 * 			99.99% of target value.
	 */
	void
	set_samples (Sample samples) noexcept;

	/**
	 * Resets smoother to initial state (or given value).
	 */
	void
	reset (float value = 0.0f) noexcept;

	/**
	 * Return smoothed sample from given input sample.
	 */
	Sample
	process (Sample s, unsigned int iterations = 1) noexcept;

	/**
	 * Smooth (low-pass) given sequence.
	 */
	template<class ForwardIterator>
		void
		process (ForwardIterator begin, ForwardIterator end) noexcept;

	/**
	 * Fill sequence with smoothed samples, where input sample is @value.
	 * \param	begin,end Sequence to be filled.
	 * \param	value Input data that is to be smoothed. It's used for each
	 * 			sample in [begin, end) sequence.
	 */
	template<class ForwardIterator>
		void
		fill (ForwardIterator begin, ForwardIterator end, Sample value) noexcept;


	/**
	 * Multiply sequence samples by given value in a smooth-way.
	 * \param	begin,end Sequence to modify.
	 * \param	value Multiplying value.
	 */
	template<class ForwardIterator>
		void
		multiply (ForwardIterator begin, ForwardIterator end, Sample value) noexcept;

  private:
	Sample
	process_single_sample (Sample s) noexcept;

  private:
	Sample _time;
	Sample _z;
};


inline
OnePoleSmoother::OnePoleSmoother (Sample samples) noexcept
{
	set_samples (samples);
	reset();
}


inline void
OnePoleSmoother::set_samples (Sample samples) noexcept
{
	_time = LookupPow::pow (0.01f, 2.0f / samples);
}


inline void
OnePoleSmoother::reset (float value) noexcept
{
	_z = value;
}


inline Sample
OnePoleSmoother::process (Sample s, unsigned int iterations) noexcept
{
	for (unsigned int i = 0; i < iterations; ++i)
		process_single_sample (s);
	return _z;
}


template<class ForwardIterator>
	inline void
	OnePoleSmoother::process (ForwardIterator begin, ForwardIterator end) noexcept
	{
		for (ForwardIterator c = begin; c != end; ++c)
			*c = process_single_sample (*c);
	}


template<class ForwardIterator>
	inline void
	OnePoleSmoother::fill (ForwardIterator begin, ForwardIterator end, Sample value) noexcept
	{
		for (ForwardIterator c = begin; c != end; ++c)
			*c = process_single_sample (value);
	}


template<class ForwardIterator>
	inline void
	OnePoleSmoother::multiply (ForwardIterator begin, ForwardIterator end, Sample value) noexcept
	{
		for (ForwardIterator c = begin; c != end; ++c)
			*c *= process_single_sample (value);
	}


inline Sample
OnePoleSmoother::process_single_sample (Sample s) noexcept
{
	return _z = _time * (_z - s) + s;
}

} // namespace DSP

} // namespace Haruhi

#endif

