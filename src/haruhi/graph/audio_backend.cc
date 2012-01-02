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
#include <haruhi/utility/atomic.h>

// Local:
#include "audio_backend.h"


namespace Haruhi {

AudioBackend::AudioBackend (std::string const& title):
	Backend ("urn://haruhi.mulabs.org/backend/audio-backend/1", title, AudioBackend::ID)
{
}


Sample
AudioBackend::master_volume() const
{
	return _master_volume.load();
}


void
AudioBackend::set_master_volume (Sample volume)
{
	_master_volume.store (volume);
}

} // namespace Haruhi

