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

// Local:
#include "../mikuru.h"
#include "waveshaper.h"


namespace MikuruPrivate {

Waveshaper::Waveshaper (int id, Mikuru* mikuru, QWidget* parent):
	Effect (parent),
	_mikuru (mikuru)
{
	_id = (id == 0) ? _mikuru->allocate_id ("waveshapers") : _mikuru->reserve_id ("waveshapers", id);
}


Waveshaper::~Waveshaper()
{
	_mikuru->free_id ("waveshapers", _id);
}

} // namespace MikuruPrivate

