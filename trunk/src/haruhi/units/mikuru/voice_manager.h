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

#ifndef HARUHI__UNITS__MIKURU__VOICE_MANAGER_H__INCLUDED
#define HARUHI__UNITS__MIKURU__VOICE_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/core/event_buffer.h>
#include <haruhi/lib/controller_param.h>

// Local:
#include "voice.h"
#include "params.h"


class Mikuru;


namespace MikuruPrivate {

class Part;


class VoiceManager
{
	/**
	 * This class is used for monophonic key priority.
	 */
	class Key
	{
	  public:
		Key (Core::Timestamp timestamp, Core::KeyID key_id, Core::VoiceEvent::Frequency frequency, Core::ControllerEvent::Value value):
			timestamp (timestamp),
			key_id (key_id),
			frequency (frequency),
			value (value)
		{ }

		bool
		operator< (Key const& other) const
		{
			return timestamp < other.timestamp;
		}

	  public:
		Core::Timestamp							timestamp;
		mutable Core::VoiceID					key_id;
		mutable Core::VoiceEvent::Frequency		frequency;
		mutable Core::ControllerEvent::Value	value;
	};

	typedef std::multiset<Key> Keys;

  public:
	VoiceManager (Part*);

	~VoiceManager();

	Part*
	part() const { return _part; }

	/**
	 * Deletes all finished voices.
	 */
	void
	sweep();

	/**
	 * Drops all voices.
	 */
	void
	panic();

	/**
	 * Unconditionally remove all voices.
	 */
	void
	hard_panic();

	/**
	 * \returns	current number of voices.
	 */
	unsigned int
	current_polyphony() const;

	/**
	 * Checks polyphony and drops excess voices.
	 */
	void
	check_polyphony();

	/**
	 * Processes new VoiceEvent.
	 */
	void
	voice_event (Core::VoiceEvent const*);

	/**
	 * Adds VoiceEvent to local buffer.
	 * It will be processed upon call to process_buffered_events().
	 */
	void
	buffer_voice_event (Core::VoiceEvent*);

	/**
	 * Processes and deletes buffered events.
	 */
	void
	process_buffered_events();

	/**
	 * Sets sustaining enabled/disabled.
	 */
	void
	set_sustain (bool enabled);

	void
	set_voice_param (Core::VoiceID, Haruhi::ControllerParam (Params::Voice::* param), int value);

	void
	set_filter1_param (Core::VoiceID, Haruhi::ControllerParam (Params::Filter::* param), int value);

	void
	set_filter2_param (Core::VoiceID, Haruhi::ControllerParam (Params::Filter::* param), int value);

	void
	set_all_voices_params (Params::Voice& params);

	void
	set_all_filters1_params (Params::Filter& params);

	void
	set_all_filters2_params (Params::Filter& params);

  private:
	void
	poly_create (Core::VoiceEvent const*);

	void
	poly_sustain (Voice*);

	void
	poly_release (Voice*);

	void
	poly_drop (Voice*);

	void
	mono_update (Core::VoiceEvent const* = 0);

	void
	mono_release();

	void
	mono_drop();

	Voice*
	find_voice_by_id (Voices& where, Core::VoiceID) const;

	void
	find_voices_by_key_id (Voices& where, Core::KeyID, Voices& result) const;

	Keys::iterator
	find_key_by_key_id (Core::VoiceID);

	Keys::iterator
	find_key_with_lowest_key_id();

	Keys::iterator
	find_key_with_highest_key_id();

	Voice*
	find_voice_by_id (Core::VoiceID) const;

	Voice*
	select_pressed_voice_to_drop() const;

	Voice*
	select_sustained_voice_to_drop() const;

	Voice*
	select_released_voice_to_drop() const;

	void
	notify_voice_created (Voice*);

	void
	notify_voice_released (Voice*);

	void
	notify_voice_dropped (Voice*);

  private:
	Mikuru*				_mikuru;
	Part*				_part;
	Keys				_keys;
	Voices				_pressed_voices;
	Voices				_sustained_voices;
	Voices				_released_voices;
	Voices				_dropped_voices;
	Voice*				_mono_voice;
	Core::EventBuffer	_events_buffer;
	bool				_sustain_notes;
};

} // namespace MikuruPrivate

#endif

