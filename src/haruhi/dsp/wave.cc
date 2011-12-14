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

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "wave.h"


namespace Haruhi {

namespace DSP {

Sample
Wave::compute_average_energy (unsigned int samples) const
{
	Sample delta = 1.0f / samples;
	Sample energy = +0.0f;
	for (Sample phase = 0.0f; phase <= 1.0f; phase += delta)
		energy += delta * (*this)(phase, 0.0f);
	return energy;
}


std::pair<Sample, Sample>
Wave::compute_min_max (unsigned int samples) const
{
	Sample delta = 1.0f / samples;
	Sample min = std::numeric_limits<Sample>::infinity();
	Sample max = -min;
	for (Sample phase = 0.0f; phase <= 1.0f; phase += delta)
	{
		Sample v = (*this)(phase, 0.0f);
		if (v < min)
			min = v;
		if (v > max)
			max = v;
	}
	return std::make_pair (min, max);
}

} // namespace DSP

} // namespace Haruhi

