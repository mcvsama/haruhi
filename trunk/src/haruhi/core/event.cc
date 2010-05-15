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
#include <cmath>

// Haruhi:
#include <haruhi/utility/pool_allocator.h>

// Local:
#include "audio.h"
#include "event.h"


namespace Core {

POOL_ALLOCATOR_FOR (Event)
POOL_ALLOCATOR_FOR (ControllerEvent)
POOL_ALLOCATOR_FOR (VoiceEvent)
POOL_ALLOCATOR_FOR (VoiceControllerEvent)


VoiceID VoiceEvent::_last_voice_id = 0;


VoiceEvent::VoiceEvent (Timestamp timestamp, KeyID key_id, VoiceID voice_id, Type type, Frequency frequency, Value value):
	Event (timestamp),
	_key_id (key_id),
	_voice_id (voice_id),
	_type (type),
	_frequency (frequency),
	_value (value)
{
	if (_voice_id == VoiceAuto)
		_voice_id = ++_last_voice_id;
	if (_frequency == 0.0)
		_frequency = VoiceEvent::frequency_from_key_id (key_id);
}


VoiceEvent::Frequency
VoiceEvent::frequency_from_key_id (KeyID key_id)
{
	return 440 * std::pow (2.0f, ((static_cast<float> (key_id) - 69.0f) / 12.0f));
}

} // namespace Core

