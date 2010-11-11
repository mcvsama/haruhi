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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/functions.h>

// Local:
#include "fourier_series_filler.h"


namespace Haruhi {

namespace DSP {

FourierSeriesFiller::FourierSeriesFiller (Function function):
	_function (function)
{
	assert (function != 0);
}


void
FourierSeriesFiller::fill (Wavetable::Tables& tables, int number, int samples)
{
	assert (number > 0);
	assert (samples > 0);

	// Create wavetables:
	for (int i = 0; i < number; ++i)
	{
		tables.push_back (new Sample[samples]);
		double const base_frequency = 0.5 - 0.5 * (std::pow (2, i) - 1) / std::pow (2, i);
		// Initialize first table or copy coefficients from previous to current table:
		if (i > 0)
			std::copy (tables[i - 1], tables[i - 1] + samples, tables[i]);
		else
			std::fill (tables[0], tables[0] + samples, 0.0);
		// Add new harmonics:
		for (int s = 0; s < samples; ++s)
			tables[i][s] += _function (2.0 * s / samples - 1.0, base_frequency, 0.2500001, 0.5);
	}
}

} // namespace DSP

} // namespace Haruhi

