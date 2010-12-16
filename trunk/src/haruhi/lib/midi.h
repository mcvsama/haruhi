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

#ifndef HARUHI__LIB__MIDI_H__INCLUDED
#define HARUHI__LIB__MIDI_H__INCLUDED

// Standard:
#include <string>
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

namespace MIDI {

enum Note
{
	NoteNone	= -1,

	NoteC_2		= 0,	NoteC_1		= 13,	NoteC0		= 25,	NoteC1		= 37,	NoteC2		= 49,	NoteC3		= 61,
	NoteCis_2	= 1,	NoteCis_1	= 14,	NoteCis0	= 26,	NoteCis1	= 38,	NoteCis2	= 50,	NoteCis3	= 62,
	NoteDes_2	= 1,	NoteDes_1	= 14,	NoteDes0	= 26,	NoteDes1	= 38,	NoteDes2	= 50,	NoteDes3	= 62,
	NoteD_2		= 2,	NoteD_1		= 15,	NoteD0		= 27,	NoteD1		= 39,	NoteD2		= 51,	NoteD3		= 63,
	NoteDis_2	= 3,	NoteDis_1	= 16,	NoteDis0	= 28,	NoteDis1	= 40,	NoteDis2	= 52,	NoteDis3	= 64,
	NoteEs_2	= 3,	NoteEs_1	= 16,	NoteEs0		= 28,	NoteEs1		= 40,	NoteEs2		= 52,	NoteEs3		= 64,
	NoteE_2		= 4,	NoteE_1		= 17,	NoteE0		= 29,	NoteE1		= 41,	NoteE2		= 53,	NoteE3		= 65,
	NoteF_2		= 6,	NoteF_1		= 18,	NoteF0		= 30,	NoteF1		= 42,	NoteF2		= 54,	NoteF3		= 66,
	NoteFis_2	= 7,	NoteFis_1	= 19,	NoteFis0	= 31,	NoteFis1	= 43,	NoteFis2	= 55,	NoteFis3	= 67,
	NoteGes_2	= 7,	NoteGes_1	= 19,	NoteGes0	= 31,	NoteGes1	= 43,	NoteGes2	= 55,	NoteGes3	= 67,
	NoteG_2		= 8,	NoteG_1		= 20,	NoteG0		= 32,	NoteG1		= 44,	NoteG2		= 56,	NoteG3		= 68,
	NoteGis_2	= 9,	NoteGis_1	= 21,	NoteGis0	= 33,	NoteGis1	= 45,	NoteGis2	= 57,	NoteGis3	= 69,
	NoteAs_2	= 9,	NoteAs_1	= 21,	NoteAs0		= 33,	NoteAs1		= 45,	NoteAs2		= 57,	NoteAs3		= 69,
	NoteA_2		= 10,	NoteA_1		= 22,	NoteA0		= 34,	NoteA1		= 46,	NoteA2		= 58,	NoteA3		= 70,
	NoteAis_2	= 11,	NoteAis_1	= 23,	NoteAis0	= 35,	NoteAis1	= 47,	NoteAis2	= 59,	NoteAis3	= 71,
	NoteBes_2	= 11,	NoteBes_1	= 23,	NoteBes0	= 35,	NoteBes1	= 47,	NoteBes2	= 59,	NoteBes3	= 71,
	NoteB_2		= 12,	NoteB_1		= 24,	NoteB0		= 36,	NoteB1		= 48,	NoteB2		= 60,	NoteB3		= 72,

	NoteC4		= 73,	NoteC5		= 85,	NoteC6		= 97,	NoteC7		= 109,	NoteC8		= 121,
	NoteCis4	= 74,	NoteCis5	= 86,	NoteCis6	= 98,	NoteCis7	= 110,	NoteCis8	= 122,
	NoteDes4	= 74,	NoteDes5	= 86,	NoteDes6	= 98,	NoteDes7	= 110,	NoteDes8	= 122,
	NoteD4		= 75,	NoteD5		= 87,	NoteD6		= 99,	NoteD7		= 111,	NoteD8		= 123,
	NoteDis4	= 76,	NoteDis5	= 88,	NoteDis6	= 100,	NoteDis7	= 112,	NoteDis8	= 124,
	NoteEs4		= 76,	NoteEs5		= 88,	NoteEs6		= 100,	NoteEs7		= 112,	NoteEs8		= 124,
	NoteE4		= 77,	NoteE5		= 89,	NoteE6		= 101,	NoteE7		= 113,	NoteE8		= 125,
	NoteF4		= 78,	NoteF5		= 90,	NoteF6		= 102,	NoteF7		= 114,	NoteF8		= 126,
	NoteFis4	= 79,	NoteFis5	= 91,	NoteFis6	= 103,	NoteFis7	= 115,	NoteFis8	= 127,
	NoteGes4	= 79,	NoteGes5	= 91,	NoteGes6	= 103,	NoteGes7	= 115,	NoteGes8	= 127,
	NoteG4		= 80,	NoteG5		= 92,	NoteG6		= 104,	NoteG7		= 116,
	NoteGis4	= 81,	NoteGis5	= 93,	NoteGis6	= 105,	NoteGis7	= 117,
	NoteAs4		= 81,	NoteAs5		= 93,	NoteAs6		= 105,	NoteAs7		= 117,
	NoteA4		= 82,	NoteA5		= 94,	NoteA6		= 106,	NoteA7		= 118,
	NoteAis4	= 83,	NoteAis5	= 95,	NoteAis6	= 107,	NoteAis7	= 119,
	NoteBes4	= 83,	NoteBes5	= 95,	NoteBes6	= 107,	NoteBes7	= 119,
	NoteB4		= 84,	NoteB5		= 96,	NoteB6		= 108,	NoteB7		= 120
};


struct Event
{
	enum Type {
		NoteOn,
		NoteOff,
		Controller,
		Pitchbend,
		ChannelPressure,
		KeyPressure,
	};

	Timestamp timestamp;
	Type type;

	union
	{
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

	/**
	 * Creates human-readable string representation
	 * of the event. Use for events lists, etc.
	 */
	std::string
	human_readable()
	{
		// TODO
		switch (type)
		{
			case NoteOn:
			case NoteOff:
				break;

			case Controller:
				break;

			case Pitchbend:
				break;

			case ChannelPressure:
				break;

			case KeyPressure:
				break;
		}

		return "";
	}
};


std::string
key_name (Note key);

} // namespace MIDI

} // namespace Haruhi

#endif
