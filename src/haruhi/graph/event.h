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

#ifndef HARUHI__GRAPH__EVENT_H__INCLUDED
#define HARUHI__GRAPH__EVENT_H__INCLUDED

// Standard:
#include <cstddef>
#include <inttypes.h>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/pool_allocator.h>
#include <haruhi/utility/memory.h>


namespace Haruhi {

// Must be int-enum to be compatible with VoiceID.
enum {
	OmniKey		= -1, // All keys.
	OmniVoice	= -1, // All voices (where applicable).
	VoiceAuto	= -1, // Voice ID will be autogenerated (unique as possible).
};

typedef int KeyID;
typedef int VoiceID;
typedef int ControllerID;


class Event: public FastShared
{
	USES_POOL_ALLOCATOR (Event)

  public:
	// Enum for descendant classes. For performance reasons
	// no dynamic_casting is done, instead use `type() const`
	// method to identify descendant class.
	enum EventType
	{
		ControllerEventType,
		VoiceEventType,
		VoiceControllerEventType,
	};

  public:
	Event (EventType type, Timestamp timestamp) noexcept;

  protected:
	// Hide copy constructor from public:
	Event (Event const& other) noexcept;

	void
	set_event_type (EventType type) noexcept;

  public:
	virtual ~Event() { }

	Timestamp
	timestamp() const noexcept;

	bool
	operator< (Event const& other) const noexcept;

	virtual Event*
	clone() const = 0;

	/**
	 * Identifies descendant class.
	 */
	EventType
	event_type() const noexcept;

  public:
	/**
	 * Used for comparison two pointer to events
	 * by their timestamp. It is Strict Weak Ordering function object.
	 */
	static bool
	strict_weak_ordering (Event const* first, Event const* second) noexcept;

	/**
	 * Same as strict_weak_ordering() but for Shared containers.
	 */
	static bool
	shared_strict_weak_ordering (Shared<Event> const& first, Shared<Event> const& second) noexcept;

  private:
	Timestamp	_timestamp;
	EventType	_event_type;
};


class ControllerEvent: public Event
{
	USES_POOL_ALLOCATOR (ControllerEvent)

  public:
	typedef float Value;

  public:
	ControllerEvent (Timestamp timestamp, Value value) noexcept;

  protected:
	ControllerEvent (ControllerEvent const& other) noexcept;

  public:
	Value
	value() const noexcept;

	ControllerEvent*
	clone() const;

  private:
	Value _value;
};


class VoiceEvent: public Event
{
	USES_POOL_ALLOCATOR (VoiceEvent)

  public:
	enum class Action {
		Create,		// Create new voice.
		Drop,		// Destroy voice.
	};

	typedef float Frequency;
	typedef ControllerEvent::Value Value;

  public:
	VoiceEvent (Timestamp timestamp, KeyID key_id, VoiceID voice_id, Action action, Frequency frequency, Value value) noexcept;

  protected:
	VoiceEvent (VoiceEvent const& other) noexcept;

  public:
	KeyID
	key_id() const noexcept;

	VoiceID
	voice_id() const noexcept;

	Action
	action() const noexcept;

	Frequency
	frequency() const noexcept;

	Value
	value() const noexcept;

	VoiceEvent*
	clone() const;

	static Frequency
	frequency_from_key_id (KeyID, float master_tune) noexcept;

  private:
	KeyID			_key_id;
	VoiceID			_voice_id;
	Action			_action;
	Frequency		_frequency;
	Value			_value;
	static VoiceID	_last_voice_id;
};


class VoiceControllerEvent: public ControllerEvent
{
	USES_POOL_ALLOCATOR (VoiceControllerEvent)

  public:
	VoiceControllerEvent (Timestamp timestamp, VoiceID voice_id, Value value) noexcept;

  protected:
	VoiceControllerEvent (VoiceControllerEvent const& other) noexcept;

  public:
	VoiceID
	voice_id() const noexcept;

	VoiceControllerEvent*
	clone() const;

  private:
	VoiceID _voice_id;
};


inline
Event::Event (EventType type, Timestamp timestamp) noexcept:
	_timestamp (timestamp),
	_event_type (type)
{ }


inline
Event::Event (Event const& other) noexcept:
	_timestamp (other._timestamp),
	_event_type (other._event_type)
{ }


inline void
Event::set_event_type (EventType type) noexcept
{
	_event_type = type;
}


inline Timestamp
Event::timestamp() const noexcept
{
	return _timestamp;
}


inline bool
Event::operator< (Event const& other) const noexcept
{
	return timestamp() < other.timestamp();
}


inline Event::EventType
Event::event_type() const noexcept
{
	return _event_type;
}


inline bool
Event::strict_weak_ordering (Event const* first, Event const* second) noexcept
{
	return first->timestamp() < second->timestamp();
}


inline bool
Event::shared_strict_weak_ordering (Shared<Event> const& first, Shared<Event> const& second) noexcept
{
	return first->timestamp() < second->timestamp();
}


inline
ControllerEvent::ControllerEvent (Timestamp timestamp, Value value) noexcept:
	Event (ControllerEventType, timestamp),
	_value (value)
{ }


inline
ControllerEvent::ControllerEvent (ControllerEvent const& other) noexcept:
	Event (other),
	_value (other._value)
{ }


inline ControllerEvent::Value
ControllerEvent::value() const noexcept
{
	return _value;
}


inline ControllerEvent*
ControllerEvent::clone() const
{
	return new ControllerEvent (*this);
}


inline
VoiceEvent::VoiceEvent (Timestamp timestamp, KeyID key_id, VoiceID voice_id, Action action, Frequency frequency, Value value) noexcept:
	Event (VoiceEventType, timestamp),
	_key_id (key_id),
	_voice_id (voice_id),
	_action (action),
	_frequency (frequency),
	_value (value)
{
	if (_voice_id == VoiceAuto)
		_voice_id = ++_last_voice_id;
}


inline
VoiceEvent::VoiceEvent (VoiceEvent const& other) noexcept:
	Event (other),
	_key_id (other._key_id),
	_voice_id (other._voice_id),
	_action (other._action),
	_frequency (other._frequency),
	_value (other._value)
{ }


inline KeyID
VoiceEvent::key_id() const noexcept
{
	return _key_id;
}


inline VoiceID
VoiceEvent::voice_id() const noexcept
{
	return _voice_id;
}


inline VoiceEvent::Action
VoiceEvent::action() const noexcept
{
	return _action;
}


inline VoiceEvent::Frequency
VoiceEvent::frequency() const noexcept
{
	return _frequency;
}


inline VoiceEvent::Value
VoiceEvent::value() const noexcept
{
	return _value;
}


inline VoiceEvent*
VoiceEvent::clone() const
{
	return new VoiceEvent (*this);
}


inline
VoiceControllerEvent::VoiceControllerEvent (Timestamp timestamp, VoiceID voice_id, Value value) noexcept:
	ControllerEvent (timestamp, value),
	_voice_id (voice_id)
{
	set_event_type (VoiceControllerEventType);
}


inline
VoiceControllerEvent::VoiceControllerEvent (VoiceControllerEvent const& other) noexcept:
	ControllerEvent (other),
	_voice_id (other._voice_id)
{ }


inline VoiceID
VoiceControllerEvent::voice_id() const noexcept
{
	return _voice_id;
}


inline VoiceControllerEvent*
VoiceControllerEvent::clone() const
{
	return new VoiceControllerEvent (*this);
}

} // namespace Haruhi

#endif

