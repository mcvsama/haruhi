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

#ifndef HARUHI__CORE__EVENT_H__INCLUDED
#define HARUHI__CORE__EVENT_H__INCLUDED

// Standard:
#include <cstddef>
#include <inttypes.h>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/pool_allocator.h>
#include <haruhi/utility/memory.h>


namespace Haruhi {

enum { OmniKey = -1, OmniVoice = -1, VoiceAuto = -1 };

typedef int KeyID;
typedef int VoiceID;
typedef int ControllerID;


class Event
{
	USES_POOL_ALLOCATOR (Event)

  public:
	// Used for comparison two pointer to events
	// by their timestamp. It is Strict Weak Ordering function object.
	class StrictWeakOrdering
	{
	  public:
		bool
		operator() (Event const* first, Event const* second) const
		{
			return first->timestamp() < second->timestamp();
		}
	};

	// Same as StrictWeakOrdering but for Shared containers.
	class SharedStrictWeakOrdering
	{
	  public:
		bool
		operator() (Shared<Event> const& first, Shared<Event> const& second) const
		{
			return first->timestamp() < second->timestamp();
		}
	};

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
	Event (Timestamp timestamp):
		_timestamp (timestamp)
	{ }

  protected:
	// Hide copy constructor from public:
	Event (Event const& other):
		_timestamp (other._timestamp)
	{ }

  public:
	virtual ~Event() { }

	Timestamp
	timestamp() const { return _timestamp; }

	bool
	operator< (Event const& other) const { return timestamp() < other.timestamp(); }

	virtual Event*
	clone() const = 0;

	/**
	 * Identifies descendant class.
	 */
	virtual EventType
	event_type() const = 0;

  private:
	Timestamp _timestamp;
};


class ControllerEvent: public Event
{
	USES_POOL_ALLOCATOR (ControllerEvent)

  public:
	typedef float Value;

  public:
	ControllerEvent (Timestamp timestamp, Value value):
		Event (timestamp),
		_value (value)
	{ }

  protected:
	ControllerEvent (ControllerEvent const& other):
		Event (other),
		_value (other._value)
	{ }

  public:
	Value
	value() const { return _value; }

	ControllerEvent*
	clone() const { return new ControllerEvent (*this); }

	EventType
	event_type() const { return ControllerEventType; }

  private:
	Value _value;
};


class VoiceEvent: public Event
{
	USES_POOL_ALLOCATOR (VoiceEvent)

  public:
	enum Type {
		Create,		// Create new voice
		Release,	// Release voice
		Drop,		// Drop voice without releasing
	};

	typedef float Frequency;
	typedef ControllerEvent::Value Value;

  public:
	VoiceEvent (Timestamp, KeyID, VoiceID, Type, Frequency, Value);

  protected:
	VoiceEvent (VoiceEvent const& other):
		Event (other),
		_key_id (other._key_id),
		_voice_id (other._voice_id),
		_type (other._type),
		_frequency (other._frequency),
		_value (other._value)
	{ }

  public:
	KeyID
	key_id() const { return _key_id; }

	VoiceID
	voice_id() const { return _voice_id; }

	Type
	type() const { return _type; }

	Frequency
	frequency() const { return _frequency; }

	Value
	value() const { return _value; }

	VoiceEvent*
	clone() const { return new VoiceEvent (*this); }

	static Frequency
	frequency_from_key_id (KeyID, float master_tune);

	EventType
	event_type() const { return VoiceEventType; }

  private:
	KeyID _key_id;
	VoiceID	_voice_id;
	Type _type;
	Frequency _frequency;
	Value _value;
	static VoiceID _last_voice_id;
};


class VoiceControllerEvent: public ControllerEvent
{
	USES_POOL_ALLOCATOR (VoiceControllerEvent)

  public:
	VoiceControllerEvent (Timestamp timestamp, VoiceID voice_id, Value value):
		ControllerEvent (timestamp, value),
		_voice_id (voice_id)
	{ }

  protected:
	VoiceControllerEvent (VoiceControllerEvent const& other):
		ControllerEvent (other),
		_voice_id (other._voice_id)
	{ }

  public:
	VoiceID
	voice_id() const { return _voice_id; }

	VoiceControllerEvent*
	clone() const { return new VoiceControllerEvent (*this); }

	EventType
	event_type() const { return VoiceControllerEventType; }

  private:
	VoiceID _voice_id;
};

} // namespace Haruhi

// Local:
#include "event_port.h"
#include "event_buffer.h"

#endif

