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
#include <haruhi/dsp/functions.h>

// Local:
#include "wavetable.h"


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
Wavetable::drop_tables()
{
	for (Tables::size_type i = 0; i < _tables.size(); ++i)
		delete[] _tables[i];
	_tables.clear();
}

} // namespace DSP

