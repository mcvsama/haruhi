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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/dsp/functions.h>

// Local:
#include "wavetable.h"


namespace Haruhi {

namespace DSP {

Wavetable::Wavetable():
	_tables(),
	_size (0)
{
}


Wavetable::~Wavetable()
{
	drop_tables();
}


void
Wavetable::add_table (Sample* samples, float max_frequency)
{
	Tables::iterator t = _tables.find (max_frequency);
	// Delete old array, if we're overwriting table:
	if (t != _tables.end())
		delete[] t->second;
	_tables[max_frequency] = samples;
}


void
Wavetable::drop_tables()
{
	for (auto& t: _tables)
		delete[] t.second;
	_tables.clear();
}

} // namespace DSP

} // namespace Haruhi

