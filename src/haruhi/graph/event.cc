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
#include <cmath>

// Haruhi:
#include <haruhi/utility/pool_allocator.h>

// Local:
#include "event.h"


namespace Haruhi {

POOL_ALLOCATOR_FOR (Event)
POOL_ALLOCATOR_FOR (ControllerEvent)
POOL_ALLOCATOR_FOR (VoiceEvent)
POOL_ALLOCATOR_FOR (VoiceControllerEvent)

VoiceID VoiceEvent::_last_voice_id = 0;


VoiceEvent::Frequency
VoiceEvent::frequency_from_key_id (KeyID key_id, float master_tune)
{
	return master_tune * std::pow (2.0f, ((static_cast<float> (key_id) - 69.0f) / 12.0f));
}

} // namespace Haruhi

