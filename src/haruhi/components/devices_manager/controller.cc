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
#include <cmath>

// Qt:
#include <QtGui/QApplication>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/lib/midi.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/qdom_sequence.h>

// Local:
#include "controller.h"


namespace Haruhi {

namespace DevicesManager {

Controller::Controller (QString const& name):
	note_filter (false),
	note_channel (0),
	controller_filter (false),
	controller_channel (0),
	controller_number (0),
	controller_invert (false),
	pitchbend_filter (false),
	pitchbend_channel (0),
	channel_pressure_filter (false),
	channel_pressure_channel (0),
	channel_pressure_invert (false),
	key_pressure_filter (false),
	key_pressure_channel (0),
	key_pressure_invert (false),
	smoothing (0),
	_name (name)
{
	for (auto& voice_id: _voice_ids)
		voice_id = 0;
}


bool
Controller::operator== (Controller const& other) const
{
	return
		note_filter == other.note_filter &&
		note_channel == other.note_channel &&
		controller_filter == other.controller_filter &&
		controller_channel == other.controller_channel &&
		controller_number == other.controller_number &&
		controller_invert == other.controller_invert &&
		pitchbend_filter == other.pitchbend_filter &&
		pitchbend_channel == other.pitchbend_channel &&
		channel_pressure_filter == other.channel_pressure_filter &&
		channel_pressure_channel == other.channel_pressure_channel &&
		channel_pressure_invert == other.channel_pressure_invert &&
		key_pressure_filter == other.key_pressure_filter &&
		key_pressure_channel == other.key_pressure_channel &&
		key_pressure_invert == other.key_pressure_invert &&
		smoothing == other.smoothing &&
		_name == other._name;
}


bool
Controller::learn_from_event (MIDI::Event const& event)
{
	switch (event.type)
	{
		case MIDI::Event::NoteOn:
		case MIDI::Event::NoteOff:
			note_filter = true;
			note_channel = (event.type == MIDI::Event::NoteOn ? event.note_on.channel : event.note_off.channel) + 1;
			controller_filter = false;
			pitchbend_filter = false;
			channel_pressure_filter = false;
			return true;

		case MIDI::Event::Controller:
			note_filter = false;
			controller_filter = true;
			controller_channel = event.controller.channel + 1;
			controller_number = event.controller.number;
			controller_invert = false;
			pitchbend_filter = false;
			channel_pressure_filter = false;
			return true;

		case MIDI::Event::Pitchbend:
			note_filter = false;
			controller_filter = false;
			pitchbend_filter = true;
			pitchbend_channel = event.pitchbend.channel + 1;
			channel_pressure_filter = false;
			return true;

		case MIDI::Event::ChannelPressure:
			note_filter = false;
			controller_filter = false;
			pitchbend_filter = false;
			channel_pressure_filter = true;
			channel_pressure_channel = event.channel_pressure.channel + 1;
			return true;

		default:
			return false;
	}
}


bool
Controller::handle_event (MIDI::Event const& midi_event, EventBuffer& buffer, Graph* graph)
{
	bool handled = false;
	Timestamp const t = midi_event.timestamp;

	switch (midi_event.type)
	{
		case MIDI::Event::NoteOn:
			if (note_filter && (note_channel == 0 || note_channel == midi_event.note_on.channel + 1))
			{
				// If there was previously note-on on that key, send voice-off:
				if (_voice_ids[midi_event.note_on.note] != OmniVoice)
				{
					float velocity = midi_event.note_on.velocity / 127.0f;
					buffer.push (new VoiceControllerEvent (t, midi_event.note_on.note, midi_event.note_on.velocity / 127.0f));
					buffer.push (new VoiceEvent (t, midi_event.note_on.note, _voice_ids[midi_event.note_on.note], VoiceEvent::Release,
												 VoiceEvent::frequency_from_key_id (midi_event.note_on.note, graph->master_tune()), velocity));
				}

				float velocity = midi_event.note_on.velocity / 127.0f;
				VoiceEvent* ve = new VoiceEvent (t, midi_event.note_on.note, VoiceAuto,
												 (midi_event.note_on.velocity == 0)? VoiceEvent::Release : VoiceEvent::Create,
												 VoiceEvent::frequency_from_key_id (midi_event.note_on.note, graph->master_tune()), velocity);
				_voice_ids[midi_event.note_on.note] = ve->voice_id();
				buffer.push (ve);
				buffer.push (new VoiceControllerEvent (t, midi_event.note_on.note, velocity));
				handled = true;
			}
			break;

		case MIDI::Event::NoteOff:
			if (note_filter && (note_channel == 0 || note_channel == midi_event.note_off.channel + 1))
			{
				float velocity = midi_event.note_off.velocity / 127.0f;
				buffer.push (new VoiceControllerEvent (t, midi_event.note_off.note, midi_event.note_off.velocity / 127.0f));
				buffer.push (new VoiceEvent (t, midi_event.note_off.note, _voice_ids[midi_event.note_off.note], VoiceEvent::Release,
											 VoiceEvent::frequency_from_key_id (midi_event.note_off.note, graph->master_tune()), velocity));
				_voice_ids[midi_event.note_off.note] = OmniVoice;
				handled = true;
			}
			break;

		case MIDI::Event::Controller:
			{
				int value = midi_event.controller.value;
				if (controller_invert)
					value = 127 - value;
				if (controller_filter && (controller_channel == 0 || controller_channel == midi_event.controller.channel + 1) && controller_number == static_cast<int> (midi_event.controller.number))
				{
					float const fvalue = value / 127.0f;
					buffer.push (new ControllerEvent (t, fvalue));
					handled = true;
					if (smoothing > 0)
						controller_smoothing_setup (t, fvalue, 0.001f, 0.001f * smoothing, graph->sample_rate());
				}
			}
			break;

		case MIDI::Event::Pitchbend:
			if (pitchbend_filter && (pitchbend_channel == 0 || pitchbend_channel == midi_event.pitchbend.channel + 1))
			{
				float const fvalue = midi_event.pitchbend.value == 0 ? 0.5f : (midi_event.pitchbend.value + 8192) / 16382.0f;
				buffer.push (new ControllerEvent (t, fvalue));
				handled = true;
				if (smoothing > 0)
					controller_smoothing_setup (t, fvalue, 0.001f, 0.001f * smoothing, graph->sample_rate());
			}
			break;

		case MIDI::Event::ChannelPressure:
			{
				int value = midi_event.channel_pressure.value;
				if (channel_pressure_invert)
					value = 127 - value;
				if (channel_pressure_filter && (channel_pressure_channel == 0 || channel_pressure_channel == midi_event.channel_pressure.channel + 1))
				{
					float const fvalue = value / 127.0f;
					buffer.push (new ControllerEvent (t, fvalue));
					handled = true;
					if (smoothing > 0)
						channel_pressure_smoothing_setup (t, fvalue, 0.001f, 0.001f * smoothing, graph->sample_rate());
				}
			}
			break;

		case MIDI::Event::KeyPressure:
			{
				int value = midi_event.key_pressure.value;
				if (key_pressure_invert)
					value = 127 - value;
				if (key_pressure_filter && (key_pressure_channel == 0 || key_pressure_channel == midi_event.key_pressure.channel + 1))
				{
					unsigned int key = bound (static_cast<unsigned int> (midi_event.key_pressure.note), 0u, 127u);
					float const fvalue = value / 127.0f;
					buffer.push (new VoiceControllerEvent (t, key, fvalue));
					handled = true;
					if (smoothing > 0)
						key_pressure_smoothing_setup (key, t, fvalue, 0.001f, 0.001f * smoothing, graph->sample_rate());
				}
			}
			break;
	}

	return handled;
}


void
Controller::generate_smoothing_events (EventBuffer& buffer, Graph* graph)
{
	if (smoothing == 0)
		return;

	Timestamp const t = graph->timestamp();

	SmoothingParams* sp_tab[] = { &_controller_smoother, &_channel_pressure_smoother };
	for (SmoothingParams* sp: sp_tab)
	{
		if (sp->current != sp->target)
		{
			// If difference is small enough, treat is as no difference:
			if (std::abs (sp->target - sp->current) < 0.001f)
				sp->current = sp->target;
			else
				sp->current = sp->smoother.process (sp->target, graph->buffer_size());
			buffer.push (new ControllerEvent (t, sp->current));
		}
	}

	for (unsigned int key = 0; key < countof (_key_pressure_smoother); ++key)
	{
		SmoothingParams& ks = _key_pressure_smoother[key];
		if (ks.current != ks.target)
		{
			if (std::abs (ks.target - ks.current) < 0.001f)
				ks.current = ks.target;
			else
				ks.current = ks.smoother.process (ks.target, graph->buffer_size());
			buffer.push (new VoiceControllerEvent (t, key, ks.current));
		}
	}
}


void
Controller::save_state (QDomElement& element) const
{
	element.setAttribute ("name", _name);
	element.setAttribute ("smoothing", smoothing);

	QDomElement note_filter_el = element.ownerDocument().createElement ("note-filter");
	note_filter_el.setAttribute ("enabled", note_filter ? "true" : "false");
	note_filter_el.setAttribute ("channel", note_channel == 0 ? "all" : QString ("%1").arg (note_channel));

	QDomElement controller_filter_el = element.ownerDocument().createElement ("controller-filter");
	controller_filter_el.setAttribute ("enabled", controller_filter ? "true" : "false");
	controller_filter_el.setAttribute ("channel", controller_filter == 0 ? "all" : QString ("%1").arg (controller_channel));
	controller_filter_el.setAttribute ("controller-number", QString ("%1").arg (controller_number));
	controller_filter_el.setAttribute ("controller-invert", controller_invert ? "true" : "false");

	QDomElement pitchbend_filter_el = element.ownerDocument().createElement ("pitchbend-filter");
	pitchbend_filter_el.setAttribute ("enabled", pitchbend_filter ? "true" : "false");
	pitchbend_filter_el.setAttribute ("channel", pitchbend_channel == 0 ? "all" : QString ("%1").arg (pitchbend_channel));

	QDomElement channel_pressure_filter_el = element.ownerDocument().createElement ("channel-pressure");
	channel_pressure_filter_el.setAttribute ("enabled", channel_pressure_filter ? "true" : "false");
	channel_pressure_filter_el.setAttribute ("channel", channel_pressure_channel == 0 ? "all" : QString ("%1").arg (channel_pressure_channel));
	channel_pressure_filter_el.setAttribute ("invert", channel_pressure_invert ? "true" : "false");

	QDomElement key_pressure_filter_el = element.ownerDocument().createElement ("key-pressure");
	key_pressure_filter_el.setAttribute ("enabled", key_pressure_filter ? "true" : "false");
	key_pressure_filter_el.setAttribute ("channel", key_pressure_channel == 0 ? "all" : QString ("%1").arg (key_pressure_channel));
	key_pressure_filter_el.setAttribute ("invert", key_pressure_invert ? "true" : "false");

	element.appendChild (note_filter_el);
	element.appendChild (controller_filter_el);
	element.appendChild (pitchbend_filter_el);
	element.appendChild (channel_pressure_filter_el);
	element.appendChild (key_pressure_filter_el);
}


void
Controller::load_state (QDomElement const& element)
{
	_name = element.attribute ("name", "<unnamed>");
	smoothing = element.attribute ("smoothing", "0").toInt();

	for (QDomElement& e: Haruhi::QDomChildElementsSequence (element))
	{
		if (e.tagName() == "note-filter")
		{
			note_filter = e.attribute ("enabled") == "true";
			note_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
		}
		else if (e.tagName() == "controller-filter")
		{
			controller_filter = e.attribute ("enabled") == "true";
			controller_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
			controller_number = e.attribute ("controller-number").toInt();
			controller_invert = e.attribute ("controller-invert") == "true";
		}
		else if (e.tagName() == "pitchbend-filter")
		{
			pitchbend_filter = e.attribute ("enabled") == "true";
			pitchbend_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
		}
		else if (e.tagName() == "channel-pressure")
		{
			channel_pressure_filter = e.attribute ("enabled") == "true";
			channel_pressure_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
			channel_pressure_invert = e.attribute ("invert") == "true";
		}
		else if (e.tagName() == "key-pressure")
		{
			key_pressure_filter = e.attribute ("enabled") == "true";
			key_pressure_channel = e.attribute ("channel") == "all" ? 0 : e.attribute ("channel").toInt();
			key_pressure_invert = e.attribute ("invert") == "true";
		}
	}
}


void
Controller::controller_smoothing_setup (Timestamp t, float target, float min_coeff, float max_coeff, unsigned int sample_rate)
{
	float dt = sample_rate * (t - _controller_smoother.prev_timestamp) / 1000.0;
	_controller_smoother.target = target;
	_controller_smoother.smoother.set_samples (bound (dt, min_coeff * sample_rate, max_coeff * sample_rate));
	_controller_smoother.prev_timestamp = t;
}


void
Controller::channel_pressure_smoothing_setup (Timestamp t, float target, float min_coeff, float max_coeff, unsigned int sample_rate)
{
	float dt = sample_rate * (t - _channel_pressure_smoother.prev_timestamp) / 1000.0;
	_channel_pressure_smoother.target = target;
	_channel_pressure_smoother.smoother.set_samples (bound (dt, min_coeff * sample_rate, max_coeff * sample_rate));
	_channel_pressure_smoother.prev_timestamp = t;
}


void
Controller::key_pressure_smoothing_setup (unsigned int key, Timestamp t, float target, float min_coeff, float max_coeff, unsigned int sample_rate)
{
	float dt = sample_rate * (t - _key_pressure_smoother[key].prev_timestamp) / 1000.0;
	_key_pressure_smoother[key].target = target;
	_key_pressure_smoother[key].smoother.set_samples (bound (dt, min_coeff * sample_rate, max_coeff * sample_rate));
	_key_pressure_smoother[key].prev_timestamp = t;
}

} // namespace DevicesManager

} // namespace Haruhi

