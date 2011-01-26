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
#include <numeric>

// Haruhi:
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/memory.h>

// Local:
#include "mikuru.h"
#include "oscillator.h"
#include "voice.h"
#include "voice_manager.h"
#include "params.h"


// TODO half-damping: when sustain is depressed, move all voices from 'released' set to 'sustained'.
// TODO Then call ->sustain() on them and notify ADSRs about resustaining().
namespace MikuruPrivate {

VoiceManager::VoiceManager (Part* part):
	_mikuru (part->mikuru()),
	_part (part),
	_mono_voice (0),
	_sustain_notes (false)
{
}


VoiceManager::~VoiceManager()
{
	hard_panic();
}


void
VoiceManager::sweep()
{
	for (Voices::iterator v = _dropped_voices.begin(); v != _dropped_voices.end(); )
	{
		if ((*v)->finished())
		{
			delete *v;
			Voices::iterator next = v;
			++next;
			_dropped_voices.erase (v);
			v = next;
		}
		else
			++v;
	}
}


void
VoiceManager::panic()
{
	Voices copy;
	_keys.clear();
	// Move all voices to 'dropped' set:
	copy = _pressed_voices;
	for (Voices::iterator v = copy.begin(); v != copy.end(); ++v)
		poly_drop (*v);
	copy = _sustained_voices;
	for (Voices::iterator v = copy.begin(); v != copy.end(); ++v)
		poly_drop (*v);
	copy = _released_voices;
	for (Voices::iterator v = copy.begin(); v != copy.end(); ++v)
		poly_drop (*v);
	mono_drop();
}


void
VoiceManager::hard_panic()
{
	for (Voices::iterator v = _pressed_voices.begin(); v != _pressed_voices.end(); ++v)
	{
		notify_voice_dropped (*v);
		delete *v;
	}
	_pressed_voices.clear();

	for (Voices::iterator v = _sustained_voices.begin(); v != _sustained_voices.end(); ++v)
	{
		notify_voice_dropped (*v);
		delete *v;
	}
	_sustained_voices.clear();

	for (Voices::iterator v = _released_voices.begin(); v != _released_voices.end(); ++v)
	{
		notify_voice_dropped (*v);
		delete *v;
	}
	_released_voices.clear();

	for (Voices::iterator v = _dropped_voices.begin(); v != _dropped_voices.end(); ++v)
	{
		notify_voice_dropped (*v);
		delete *v;
	}
	_dropped_voices.clear();

	if (_mono_voice)
	{
		notify_voice_dropped (_mono_voice);
		delete _mono_voice;
		_mono_voice = 0;
	}
}


unsigned int
VoiceManager::current_polyphony() const
{
	return _pressed_voices.size() + _sustained_voices.size() + _released_voices.size() + (atomic (_mono_voice) ? 1 : 0);
}


void
VoiceManager::check_polyphony()
{
	while (current_polyphony() > _mikuru->general()->params()->polyphony.get())
	{
		Voice* v = select_released_voice_to_drop();
		if (!v)
			v = select_sustained_voice_to_drop();
		if (!v)
			v = select_pressed_voice_to_drop();
		if (v)
			poly_drop (v);
	}
}


void
VoiceManager::voice_event (Haruhi::VoiceEvent const* voice_event)
{
	if (voice_event->type() == Haruhi::VoiceEvent::Release || voice_event->type() == Haruhi::VoiceEvent::Drop)
	{
		// Find and remove Key from _keys:
		Keys::iterator k = find_key_by_key_id (voice_event->key_id());
		while (k != _keys.end())
		{
			_keys.erase (k);
			k = find_key_by_key_id (voice_event->key_id());
		}

		if (_mono_voice)
			mono_update (voice_event);

		switch (voice_event->type())
		{
			case Haruhi::VoiceEvent::Release:
			{
				Voices result;
				find_voices_by_key_id (_pressed_voices, voice_event->key_id(), result);
				if (_sustain_notes)
				{
					for (Voices::iterator v = result.begin(); v != result.end(); ++v)
						poly_sustain (*v);
				}
				else
				{
					for (Voices::iterator v = result.begin(); v != result.end(); ++v)
						poly_release (*v);
				}
				break;
			}

			case Haruhi::VoiceEvent::Drop:
				{
					Voice* voice;
					while ((voice = find_voice_by_id (_pressed_voices, voice_event->voice_id())))
						poly_drop (voice);
					while ((voice = find_voice_by_id (_sustained_voices, voice_event->voice_id())))
						poly_drop (voice);
					while ((voice = find_voice_by_id (_released_voices, voice_event->voice_id())))
						poly_drop (voice);
				}
				break;

			default:
				;
		}
	}
	else if (voice_event->type() == Haruhi::VoiceEvent::Create && _part->params()->enabled && _part->oscillator()->wavetable())
	{
		_keys.insert (Key (voice_event->timestamp(), voice_event->key_id(), voice_event->frequency(), voice_event->value()));

		if (_part->oscillator()->oscillator_params()->monophonic.get())
			mono_update (voice_event);
		else
			poly_create (voice_event);
	}
}


void
VoiceManager::buffer_voice_event (Haruhi::VoiceEvent* voice_event)
{
	_events_buffer.push (voice_event);
}


void
VoiceManager::process_buffered_events()
{
	for (Haruhi::EventBuffer::EventsMultiset::iterator e = _events_buffer.events().begin(); e != _events_buffer.events().end(); ++e)
		voice_event (static_cast<Haruhi::VoiceEvent const*> (e->get()));
	_events_buffer.clear();
}


void
VoiceManager::set_sustain (bool enabled)
{
	_sustain_notes = enabled;
	// Sustaining disabled -> check for voices to release:
	if (!enabled)
	{
		// Move voices to 'released' set:
		Voices copy = _sustained_voices;
		for (Voices::iterator v = copy.begin(); v != copy.end(); ++v)
			poly_release (*v);
		// Update monophonic (it will drop voice if no key is pressed):
		mono_update();
	}
}


void
VoiceManager::set_voice_param (Haruhi::VoiceID voice_id, Haruhi::ControllerParam (Params::Voice::* param), int value)
{
	if (voice_id == Haruhi::OmniVoice)
	{
		Voices::iterator v;
		for (v = _pressed_voices.begin(); v != _pressed_voices.end(); ++v)
			((*v)->params()->*param).set (value);
		for (v = _sustained_voices.begin(); v != _sustained_voices.end(); ++v)
			((*v)->params()->*param).set (value);
		for (v = _released_voices.begin(); v != _released_voices.end(); ++v)
			((*v)->params()->*param).set (value);
		if (_mono_voice)
			(_mono_voice->params()->*param).set (value);
	}
	else
	{
		Voice* voice = find_voice_by_id (_pressed_voices, voice_id);
		if (voice)
			(voice->params()->*param).set (value);
		voice = find_voice_by_id (_sustained_voices, voice_id);
		if (voice)
			(voice->params()->*param).set (value);
		voice = find_voice_by_id (_released_voices, voice_id);
		if (voice)
			(voice->params()->*param).set (value);
		if (_mono_voice && _mono_voice->voice_id() == voice_id)
			(_mono_voice->params()->*param).set (value);
	}
}


void
VoiceManager::set_filter1_param (Haruhi::VoiceID voice_id, Haruhi::ControllerParam (Params::Filter::* param), int value)
{
	if (voice_id == Haruhi::OmniVoice)
	{
		Voices::iterator v;
		for (v = _pressed_voices.begin(); v != _pressed_voices.end(); ++v)
			((*v)->filter1_params()->*param).set (value);
		for (v = _sustained_voices.begin(); v != _sustained_voices.end(); ++v)
			((*v)->filter1_params()->*param).set (value);
		for (v = _released_voices.begin(); v != _released_voices.end(); ++v)
			((*v)->filter1_params()->*param).set (value);
		if (_mono_voice)
			(_mono_voice->filter1_params()->*param).set (value);
	}
	else
	{
		Voice* voice = find_voice_by_id (_pressed_voices, voice_id);
		if (voice)
			(voice->filter1_params()->*param).set (value);
		voice = find_voice_by_id (_sustained_voices, voice_id);
		if (voice)
			(voice->filter1_params()->*param).set (value);
		voice = find_voice_by_id (_released_voices, voice_id);
		if (voice)
			(voice->filter1_params()->*param).set (value);
		if (_mono_voice && _mono_voice->voice_id() == voice_id)
			(_mono_voice->filter1_params()->*param).set (value);
	}
}


void
VoiceManager::set_filter2_param (Haruhi::VoiceID voice_id, Haruhi::ControllerParam (Params::Filter::* param), int value)
{
	if (voice_id == Haruhi::OmniVoice)
	{
		Voices::iterator v;
		for (v = _pressed_voices.begin(); v != _pressed_voices.end(); ++v)
			((*v)->filter2_params()->*param).set (value);
		for (v = _sustained_voices.begin(); v != _sustained_voices.end(); ++v)
			((*v)->filter2_params()->*param).set (value);
		for (v = _released_voices.begin(); v != _released_voices.end(); ++v)
			((*v)->filter2_params()->*param).set (value);
		if (_mono_voice)
			(_mono_voice->filter2_params()->*param).set (value);
	}
	else
	{
		Voice* voice = find_voice_by_id (_pressed_voices, voice_id);
		if (voice)
			(voice->filter2_params()->*param).set (value);
		voice = find_voice_by_id (_sustained_voices, voice_id);
		if (voice)
			(voice->filter2_params()->*param).set (value);
		voice = find_voice_by_id (_released_voices, voice_id);
		if (voice)
			(voice->filter2_params()->*param).set (value);
		if (_mono_voice && _mono_voice->voice_id() == voice_id)
			(_mono_voice->filter2_params()->*param).set (value);
	}
}


void
VoiceManager::set_all_voices_params (Params::Voice& params)
{
	for (Voices::iterator v = _pressed_voices.begin(); v != _pressed_voices.end(); ++v)
		*(*v)->params() = params;
	for (Voices::iterator v = _sustained_voices.begin(); v != _sustained_voices.end(); ++v)
		*(*v)->params() = params;
	for (Voices::iterator v = _released_voices.begin(); v != _released_voices.end(); ++v)
		*(*v)->params() = params;
	if (_mono_voice)
		*_mono_voice->params() = params;
}


void
VoiceManager::set_all_filters1_params (Params::Filter& params)
{
	for (Voices::iterator v = _pressed_voices.begin(); v != _pressed_voices.end(); ++v)
		*(*v)->filter1_params() = params;
	for (Voices::iterator v = _sustained_voices.begin(); v != _sustained_voices.end(); ++v)
		*(*v)->filter1_params() = params;
	for (Voices::iterator v = _released_voices.begin(); v != _released_voices.end(); ++v)
		*(*v)->filter1_params() = params;
	if (_mono_voice)
		*_mono_voice->filter1_params() = params;
}


void
VoiceManager::set_all_filters2_params (Params::Filter& params)
{
	for (Voices::iterator v = _pressed_voices.begin(); v != _pressed_voices.end(); ++v)
		*(*v)->filter2_params() = params;
	for (Voices::iterator v = _sustained_voices.begin(); v != _sustained_voices.end(); ++v)
		*(*v)->filter2_params() = params;
	for (Voices::iterator v = _released_voices.begin(); v != _released_voices.end(); ++v)
		*(*v)->filter2_params() = params;
	if (_mono_voice)
		*_mono_voice->filter2_params() = params;
}


void
VoiceManager::poly_create (Haruhi::VoiceEvent const* voice_event)
{
	// If there is already voice with the same voice_id, drop it first:
	Voice* v = find_voice_by_id (_pressed_voices, voice_event->voice_id());
	if (v)
		poly_release (v);
	// New voice:
	Voice* voice = new Voice (this, _mikuru->select_thread_for_new_voice(), voice_event->key_id(), voice_event->voice_id(),
							  voice_event->frequency() / _mikuru->graph()->sample_rate(), voice_event->value(), voice_event->timestamp());
	_pressed_voices.insert (voice);
	notify_voice_created (voice);
}


void
VoiceManager::poly_sustain (Voice* voice)
{
	_pressed_voices.erase (voice);
	_sustained_voices.insert (voice);
}


void
VoiceManager::poly_release (Voice* voice)
{
	if (!voice->tracked())
		poly_drop (voice);
	else
	{
		_pressed_voices.erase (voice);
		_sustained_voices.erase (voice);
		_released_voices.insert (voice);
		voice->release();
		notify_voice_released (voice);
	}
}


void
VoiceManager::poly_drop (Voice* voice)
{
	_pressed_voices.erase (voice);
	_sustained_voices.erase (voice);
	_released_voices.erase (voice);
	_dropped_voices.insert (voice);
	voice->drop();
	notify_voice_dropped (voice);
}


void
VoiceManager::mono_update (Haruhi::VoiceEvent const* voice_event)
{
	if (_part->oscillator()->oscillator_params()->monophonic.get() || _mono_voice)
	{
		if (_keys.size() > 0)
		{
			Key const* key = 0;
			switch (_part->oscillator()->oscillator_params()->monophonic_key_priority.get())
			{
				case Params::Oscillator::FirstPressed:		key = &*_keys.begin(); break;
				case Params::Oscillator::LastPressed:		key = &*_keys.rbegin(); break;
				case Params::Oscillator::LowestPressed:		key = &*find_key_with_lowest_key_id(); break;
				case Params::Oscillator::HighestPressed:	key = &*find_key_with_highest_key_id(); break;
			}

			if (key)
			{
				Haruhi::KeyID key_id = key->key_id;
				Haruhi::VoiceID voice_id = voice_event ? voice_event->voice_id() : _mono_voice->voice_id();
				Haruhi::VoiceEvent::Frequency frequency = Haruhi::VoiceEvent::frequency_from_key_id (key_id, _mikuru->graph()->master_tune()) / _mikuru->graph()->sample_rate();
				Haruhi::Timestamp timestamp = key->timestamp;
				Sample value = key->value;

				if (_mono_voice)
				{
					if (_part->oscillator()->oscillator_params()->monophonic_retrigger.get() && _mono_voice->key_id() != key_id)
					{
						mono_release();
						_mono_voice = new Voice (this, _mikuru->select_thread_for_new_voice(), key_id, voice_id, frequency, value, timestamp);
						notify_voice_created (_mono_voice);
					}
					else
						_mono_voice->set_frequency (frequency);
				}
				else
				{
					_mono_voice = new Voice (this, _mikuru->select_thread_for_new_voice(), key_id, voice_id, frequency, value, timestamp);
					notify_voice_created (_mono_voice);
				}
			}
		}
		else
		{
			if (!_sustain_notes)
				mono_release();
		}
	}
}


void
VoiceManager::mono_release()
{
	if (_mono_voice)
	{
		if (!_mono_voice->tracked())
			mono_drop();
		else
		{
			_released_voices.insert (_mono_voice);
			_mono_voice->release();
			notify_voice_released (_mono_voice);
			_mono_voice = 0;
		}
	}
}


void
VoiceManager::mono_drop()
{
	if (_mono_voice)
	{
		_dropped_voices.insert (_mono_voice);
		_mono_voice->drop();
		notify_voice_dropped (_mono_voice);
		_mono_voice = 0;
	}
}


Voice*
VoiceManager::find_voice_by_id (Voices& where, Haruhi::VoiceID voice_id) const
{
	for (Voices::const_iterator v = where.begin(); v != where.end(); ++v)
		if ((*v)->voice_id() == voice_id)
			return *v;
	return 0;
}


void
VoiceManager::find_voices_by_key_id (Voices& where, Haruhi::KeyID key_id, Voices& result) const
{
	for (Voices::const_iterator v = where.begin(); v != where.end(); ++v)
		if ((*v)->key_id() == key_id)
			result.insert (*v);
}


VoiceManager::Keys::iterator
VoiceManager::find_key_by_key_id (Haruhi::KeyID key_id)
{
	for (Keys::iterator k = _keys.begin(); k != _keys.end(); ++k)
		if (k->key_id == key_id)
			return k;
	return _keys.end();
}


VoiceManager::Keys::iterator
VoiceManager::find_key_with_lowest_key_id()
{
	if (_keys.empty())
		return _keys.end();

	Keys::iterator current = _keys.begin();
	Haruhi::KeyID key_id = current->key_id;
	for (Keys::iterator k = _keys.begin(); k != _keys.end(); ++k)
	{
		if (k->key_id < key_id)
		{
			key_id = k->key_id;
			current = k;
		}
	}
	return current;
}


VoiceManager::Keys::iterator
VoiceManager::find_key_with_highest_key_id()
{
	if (_keys.empty())
		return _keys.end();

	Keys::iterator current = _keys.begin();
	Haruhi::KeyID key_id = current->key_id;
	for (Keys::iterator k = _keys.begin(); k != _keys.end(); ++k)
	{
		if (k->key_id > key_id)
		{
			key_id = k->key_id;
			current = k;
		}
	}
	return current;
}


Voice*
VoiceManager::select_pressed_voice_to_drop() const
{
	if (_pressed_voices.empty())
		return 0;
	// Find oldest voice:
	return std::accumulate (_pressed_voices.begin(), _pressed_voices.end(), *_pressed_voices.begin(), Voice::ReturnOlder());
}


Voice*
VoiceManager::select_sustained_voice_to_drop() const
{
	if (_sustained_voices.empty())
		return 0;
	// Find oldest voice:
	return std::accumulate (_sustained_voices.begin(), _sustained_voices.end(), *_sustained_voices.begin(), Voice::ReturnOlder());
}


Voice*
VoiceManager::select_released_voice_to_drop() const
{
	if (_released_voices.empty())
		return 0;
	// Find oldest voice:
	return std::accumulate (_released_voices.begin(), _released_voices.end(), *_released_voices.begin(), Voice::ReturnOlder());
}


void
VoiceManager::notify_voice_created (Voice* voice)
{
	_mikuru->general()->envelopes()->notify_voice_created (this, voice);
}


void
VoiceManager::notify_voice_released (Voice* voice)
{
	_mikuru->general()->envelopes()->notify_voice_released (this, voice);
}


void
VoiceManager::notify_voice_dropped (Voice* voice)
{
	_mikuru->general()->envelopes()->notify_voice_dropped (this, voice);
}

} // namespace MikuruPrivate

