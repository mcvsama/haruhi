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

#ifndef HARUHI__DSP__WAVETABLE_H__INCLUDED
#define HARUHI__DSP__WAVETABLE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>
#include <algorithm>
#include <map>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/utility/numeric.h>


namespace Haruhi {

namespace DSP {

class Wavetable
{
  public:
	// Maps frequency to array of samples. Frequency is max frequency for which
	// given table can be used. Use lower_bound method to find correct table.
	typedef std::map<float, Sample*>  Tables;

	/**
	 * All filler classes (that is classes that fill wavetables with samples)
	 * should inherit this class.
	 */
	class Filler
	{
	};

	/**
	 * Adapter that allows using Wavetable as a Wave.
	 * It's inefficient, but can be used for eg. drawing plots of Wavetable.
	 */
	class WaveAdapter: public Wave
	{
	  public:
		WaveAdapter (Wavetable* wavetable);

		Sample
		operator() (Sample register phase, Sample frequency) const;

	  private:
		Wavetable* _wavetable;
	};

  public:
	Wavetable();

	~Wavetable();

	/**
	 * Adds wavetable.
	 * \param	table Array of samples. Wavetable takes ownership of the pointer.
	 * \param	max_frequency Maximum frequency for which this array can be used.
	 */
	void
	add_table (Sample* samples, float max_frequency);

	/**
	 * Deletes previously allocated tables using delete operator.
	 */
	void
	drop_tables();

	/**
	 * Sets new number of samples in wavetables.
	 * Must be called by filler.
	 */
	void
	set_wavetables_size (std::size_t size);

	/**
	 * There must be at least one table added. Otherwise behavior of this
	 * method is undefined.
	 */
	Sample
	operator() (Sample register phase, Sample frequency) const;

  private:
	/**
	 * Returns wavetable index to use for given frequency.
	 * There must be at least one table in set.
	 */
	Sample const*
	table_for_frequency (float frequency) const;

  private:
	Tables			_tables;
	// Number of samples in each table:
	std::size_t		_size;
};


inline
Wavetable::WaveAdapter::WaveAdapter (Wavetable* wavetable):
	Wave (true),
	_wavetable (wavetable)
{ }


inline Sample
Wavetable::WaveAdapter::operator() (Sample phase, Sample frequency) const
{
	return _wavetable->operator() (phase, frequency);
}


inline void
Wavetable::set_wavetables_size (std::size_t size)
{
	_size = size;
}


inline Sample
Wavetable::operator() (Sample phase, Sample frequency) const
{
	Sample const* table = table_for_frequency (frequency);
	const float p = mod1 (phase) * _size;
	const int k = static_cast<int> (p);
	const Sample v1 = table[k];
	const Sample v2 = table[(k + 1) % _size];
	// Linear approximation:
	return v1 + (p - k) * (v2 - v1);
}


inline Sample const*
Wavetable::table_for_frequency (float frequency) const
{
	Tables::const_iterator t = _tables.lower_bound (frequency);
	if (t == _tables.end())
		return _tables.rbegin()->second;
	return t->second;
}

} // namespace DSP

} // namespace Haruhi

#endif

