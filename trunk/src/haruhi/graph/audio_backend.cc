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
#include <algorithm>
#include <functional>
#include <memory.h>

// Haruhi:
#include <haruhi/utility/atomic.h>

// Local:
#include "audio_backend.h"


namespace Haruhi {

Sample
AudioBackend::master_volume() const
{
	return atomic (_master_volume);
}


void
AudioBackend::set_master_volume (Sample volume)
{
	atomic (_master_volume) = volume;
}

} // namespace Haruhi

