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

#ifndef HARUHI__DSP__WAVE_H__INCLUDED
#define HARUHI__DSP__WAVE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Haruhi:
#include <haruhi/core/audio.h>


namespace DSP {

class Wave
{
  public:
	Wave();

	virtual ~Wave();

	/**
	 * Returns function's sample.
	 * \param	phase is the phase in range [0, 1].
	 * \param	frequency is the base frequency of the signal this sample will
	 * 			be used in (this is for limiting bandwidth).
	 */
	virtual Core::Sample
	operator() (Core::Sample register phase, Core::Sample frequency) const = 0;
};

} // namespace DSP

#endif

