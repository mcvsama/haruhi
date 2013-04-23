/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__LIB__MIDI_H__INCLUDED
#define HARUHI__LIB__MIDI_H__INCLUDED

// Standard:
#include <string>
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/timestamp.h>


namespace Haruhi {

namespace MIDI {

struct Event
{
  public:
	enum Type {
		NoteOn,
		NoteOff,
		Controller,
		Pitchbend,
		ChannelPressure,
		KeyPressure,
	};

	typedef uint64_t ID;

  public:
	Event():
		id (++_last_id)
	{ }

  public:
	Timestamp	timestamp;
	Type		type;
	ID			id; // Unique identifier.

	union {
		struct {
			uint8_t channel;
			uint8_t note;
			uint8_t velocity;
		} note_on;

		struct {
			uint8_t channel;
			uint8_t note;
			uint8_t velocity;
		} note_off;

		struct {
			uint8_t channel;
			uint8_t number;
			uint8_t value;
		} controller;

		struct {
			uint8_t channel;
			int16_t value;
		} pitchbend;

		struct {
			uint8_t channel;
			uint8_t value;
		} channel_pressure;

		struct {
			uint8_t channel;
			uint8_t note;
			uint8_t value;
		} key_pressure;
	};

  private:
	static ID _last_id;
};

} // namespace MIDI

} // namespace Haruhi

#endif
