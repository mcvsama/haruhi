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

#ifndef HARUHI__DSP__RAMP_SMOOTHER_H__INCLUDED
#define HARUHI__DSP__RAMP_SMOOTHER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <algorithm>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>


namespace Haruhi {

namespace DSP {

class RampSmoother
{
  public:
	RampSmoother (unsigned int max_samples = 0);

	void
	set_samples (unsigned int samples);

	void
	set_value (float value);

	void
	reset (float value);

	float
	current() const;

	float
	advance (unsigned int samples = 1);

	template<class ForwardIterator>
		void
		fill (ForwardIterator begin, ForwardIterator end, Sample value);

	template<class ForwardIterator>
		void
		multiply (ForwardIterator begin, ForwardIterator end, Sample value);

  private:
	void
	set_actual_samples (unsigned int samples);

  private:
	float			_current;
	float			_vector;
	float			_target;
	unsigned int	_samples;
	unsigned int	_samples_from_last_setup;
	unsigned int	_samples_max;
	bool			_first_value_initialized;
};


inline void
RampSmoother::set_samples (unsigned int samples)
{
	_samples_max = samples;
}


inline float
RampSmoother::current() const
{
	return _current;
}


inline float
RampSmoother::advance (unsigned int samples)
{
	_current += samples * _vector;
	_samples_from_last_setup += samples;
	if ((_vector > 0.0f && _current > _target) || (_vector < 0.0f && _current < _target))
	{
		_current = _target;
		_vector = +0.0;
	}
	return _current;
}


template<class ForwardIterator>
	inline void
	RampSmoother::fill (ForwardIterator begin, ForwardIterator end, Sample value)
	{
		set_value (value);
		for (ForwardIterator current = begin; current != end; ++current)
			*current = advance (1);
	}


template<class ForwardIterator>
	inline void
	RampSmoother::multiply (ForwardIterator begin, ForwardIterator end, Sample value)
	{
		set_value (value);
		for (ForwardIterator current = begin; current != end; ++current)
			*current *= advance (1);
	}


inline void
RampSmoother::set_actual_samples (unsigned int samples)
{
	_samples = std::max (samples, 1u);
}

} // namespace DSP

} // namespace Haruhi

#endif

