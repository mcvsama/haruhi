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

#ifndef HARUHI__DSP__NOISE_H__INCLUDED
#define HARUHI__DSP__NOISE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdlib>
#include <stdint.h>
#include <map>

// Haruhi:
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/thread.h>

// Local:
#include "wave.h"


namespace DSP {

class Noise: public Wave
{
  public:
	class State
	{
		friend class Noise;

	  public:
		State();

	  private:
		int32_t w;
	};

	typedef std::map<Thread::ID, State> States;

  public:
	Noise() { }

	/**
	 * Returns sample from base function with added harmonics.
	 * \param	phase is ignored.
	 * \param	frequency is ignored.
	 */
	virtual Core::Sample
	operator() (Core::Sample register phase, Core::Sample frequency) const
	{
		return get();
	}

	/**
	 * Faster getter of noise value (not polymorphic).
	 */
	Core::Sample
	get() const
	{
		return get (_states[Thread::id()]);
	}

	/**
	 * Faster equivalent of get().
	 */
	Core::Sample
	get (State& s) const
	{
		s.w *= 16807;
		return s.w * 4.6566129e-010f;
	}

	/**
	 * Returns reference to State object for calling thread.
	 */
	State&
	state()
	{
		// Find or create state for current thread:
		return _states[Thread::id()];
	}

  private:
	States mutable _states;
};

} // namespace DSP

#endif

