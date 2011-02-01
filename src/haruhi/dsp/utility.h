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

#ifndef HARUHI__DSP__UTILITY_H__INCLUDED
#define HARUHI__DSP__UTILITY_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>


inline float
log_meter (float db, float lower_db = -70.0, float upper_db = 6.0)
{
	return db < lower_db ? 0.0 : std::pow ((db - lower_db) / (upper_db - lower_db), 2.0);
}

#endif
