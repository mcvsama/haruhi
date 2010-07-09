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

#ifndef HARUHI__DSP__WAVETABLE_H__INCLUDED
#define HARUHI__DSP__WAVETABLE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>
#include <algorithm>

// Haruhi:
#include <haruhi/core/audio.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/utility/numeric.h>


namespace DSP {

class Wavetable
{
  public:
	enum {
		Number = 12, // Number of tables.
	};

  public:
	typedef std::vector<Core::Sample*>  Tables;

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
		WaveAdapter (Wavetable* wavetable):
			_wavetable (wavetable)
		{ }

		Core::Sample
		operator() (Core::Sample register phase, Core::Sample frequency) const
		{
			return _wavetable->operator() (phase, frequency);
		}

	  private:
		Wavetable* _wavetable;
	};

  public:
	Wavetable();

	~Wavetable();

	Tables&
	tables() { return _tables; }

	Tables const&
	tables() const { return _tables; }

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
	set_wavetables_size (std::size_t size)
	{
		_size = size;
	}

	/*
	 * Inherited from Wave.
	 */

	Core::Sample
	operator() (Core::Sample register phase, Core::Sample frequency) const
	{
		const int t = table_index (frequency);
		const float p = mod1 (phase) * _size;
		const int k = static_cast<int> (p);
		const Tables::value_type& table = _tables[t];
		const Core::Sample v1 = table[k];
		const Core::Sample v2 = table[(k + 1) % _size];
		// Linear approximation:
		return v1 + (p - k) * (v2 - v1);
	}

	Core::Sample
	base (Core::Sample register phase, Core::Sample frequency) const
	{
		return this->operator() (phase, frequency);
	}

  private:
	/**
	 * Returns wavetable index to use for given frequency.
	 */
	int
	table_index (float frequency) const
	{
		const int i = -(roundf (-log2f (1.45f * 0.5f / frequency)) + 1.0f);
		if (i < 0)
			return 0;
		else if (static_cast<unsigned int> (i) > _tables.size() - 1)
			return _tables.size() - 1;
		return i;
	}

  private:
	Tables			_tables;
	// Number of samples in each table:
	std::size_t		_size;
};

} // namespace DSP

#endif

