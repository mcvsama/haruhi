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

#ifndef HARUHI__DSP__PARAMETRIC_WAVE_H__INCLUDED
#define HARUHI__DSP__PARAMETRIC_WAVE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Haruhi:
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
	ParametricWave (bool immutable);

	/**
	 * \threadsafe
	 */
	void
	set_param (float param) { atomic (_param) = param; }

	/**
	 * \threadsafe
	 */
	float
	param() const { return atomic (_param); }

  private:
	float _param;
};

} // namespace DSP

} // namespace Haruhi

#endif

