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

#ifndef HARUHI__DSP__SMOOTHER_H__INCLUDED
#define HARUHI__DSP__SMOOTHER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <algorithm>

// Haruhi:
#include <haruhi/dsp/wave.h>


namespace Haruhi {

namespace DSP {

class Smoother
{
  public:
	Smoother (unsigned int max_samples = 0);

	void
	set_smoothing_samples (unsigned int samples);

	void
	set_value (float value);

	void
	set_absolute_value (float value);

	float
	current() const;

	float
	advance (unsigned int samples = 1);

	template<class ForwardIterator>
		void
		fill (ForwardIterator begin, ForwardIterator end);

	template<class ForwardIterator>
		void
		multiply (ForwardIterator begin, ForwardIterator end);

  private:
	void
	set_samples (unsigned int samples);

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
Smoother::set_smoothing_samples (unsigned int samples)
{
	_samples_max = samples;
}


inline float
Smoother::current() const
{
	return _current;
}


inline float
Smoother::advance (unsigned int samples)
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
	Smoother::fill (ForwardIterator begin, ForwardIterator end)
	{
		for (ForwardIterator current = begin; current != end; ++current)
			*current = advance (1);
	}


template<class ForwardIterator>
	inline void
	Smoother::multiply (ForwardIterator begin, ForwardIterator end)
	{
		for (ForwardIterator current = begin; current != end; ++current)
			*current *= advance (1);
	}


inline void
Smoother::set_samples (unsigned int samples)
{
	_samples = std::max (samples, 1u);
}

} // namespace DSP

} // namespace Haruhi

#endif

