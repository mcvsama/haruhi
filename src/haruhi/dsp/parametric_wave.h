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

#ifndef HARUHI__DSP__PARAMETRIC_WAVE_H__INCLUDED
#define HARUHI__DSP__PARAMETRIC_WAVE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/utility/atomic.h>


namespace Haruhi {

namespace DSP {

/**
 * A Wave that has one parameter to be controlled (shape modifier).
 */
class ParametricWave: public Wave
{
  public:
	ParametricWave (bool immutable) noexcept;

	/**
	 * Return clone of the wave.
	 */
	virtual ParametricWave*
	clone() const = 0;

	/**
	 * \threadsafe
	 */
	void
	set_param (float param) noexcept;

	/**
	 * \threadsafe
	 */
	float
	param() const noexcept;

  private:
	Atomic<float> _param;
};


inline
ParametricWave::ParametricWave (bool immutable) noexcept:
	Wave (immutable),
	_param (0.0)
{ }


inline void
ParametricWave::set_param (float param) noexcept
{
	_param.store (param);
}


inline float
ParametricWave::param() const noexcept
{
	return _param.load();
}

} // namespace DSP

} // namespace Haruhi

#endif

