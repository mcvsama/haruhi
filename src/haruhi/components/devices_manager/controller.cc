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
#include <haruhi/utility/qdom.h>
#include <haruhi/utility/frequency.h>

// Local:
#include "controller.h"
#include "device.h"


namespace Haruhi {

namespace DevicesManager {

Controller::Controller (QString const& name):
	_name (name)
{
}


bool
Controller::operator== (Controller const& other) const
{
	return
		note_filter == other.note_filter &&
		note_channel == other.note_channel &&
		note_pitch_filter == other.note_pitch_filter &&
		note_pitch_channel == other.note_pitch_channel &&
		note_velocity_filter == other.note_velocity_filter &&
		note_velocity_channel == other.note_velocity_channel &&
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
			reset_filters();
			note_filter = true;
			note_channel = (event.type == MIDI::Event::NoteOn ? event.note_on.channel : event.note_off.channel) + 1;
			note_pitch_channel = note_channel;
			note_velocity_channel = note_channel;
			return true;

		case MIDI::Event::Controller:
			reset_filters();
			controller_filter = true;
			controller_channel = event.controller.channel + 1;
			controller_number = event.controller.number;
			controller_invert = false;
			return true;

		case MIDI::Event::Pitchbend:
			reset_filters();
			pitchbend_filter = true;
			pitchbend_channel = event.pitchbend.channel + 1;
			return true;

		case MIDI::Event::ChannelPressure:
			reset_filters();
			channel_pressure_filter = true;
			channel_pressure_invert = false;
			channel_pressure_channel = event.channel_pressure.channel + 1;
			return true;

		case MIDI::Event::KeyPressure:
			reset_filters();
			key_pressure_filter = true;
			key_pressure_invert = false;
			key_pressure_channel = event.key_pressure.channel + 1;
			return true;

		default:
			return false;
	}
}


bool
Controller::handle_event (MIDI::Event const& midi_event, Device& device, EventBuffer& buffer, Graph* graph)
{
	bool handled = false;
	Timestamp const& t = midi_event.timestamp;

	switch (midi_event.type)
	{
		case MIDI::Event::NoteOn:
		{
			if (device._last_midi_event_id != midi_event.id)
			{
				device._allocated_voice_id = VoiceEvent::allocate_voice_id();
				device._last_midi_event_id = midi_event.id;
			}

			float velocity = midi_event.note_on.velocity / 127.0f;

			if (note_filter && (note_channel == 0 || note_channel == midi_event.note_on.channel + 1))
			{
				// If there was previously note-on on that key, send voice-off:
				if (device._voice_ids[midi_event.note_on.note] != OmniVoice)
					buffer.push (new VoiceEvent (t, midi_event.note_on.note, device._voice_ids[midi_event.note_on.note], VoiceEvent::Action::Drop));

				device._voice_ids[midi_event.note_on.note] = device._allocated_voice_id;
				buffer.push (new VoiceEvent (t, midi_event.note_on.note, device._allocated_voice_id, VoiceEvent::Action::Create));
				handled = true;
			}

			if (note_velocity_filter && (note_velocity_channel == 0 || note_velocity_channel == midi_event.note_on.channel + 1))
			{
				buffer.push (new VoiceControllerEvent (t, device._allocated_voice_id, velocity));
				handled = true;
			}

			if (note_pitch_filter && (note_pitch_channel == 0 || note_pitch_channel == midi_event.note_on.channel + 1))
			{
				buffer.push (new VoiceControllerEvent (t, device._allocated_voice_id,
							 static_cast<Haruhi::ControllerEvent::Value> (VoiceEvent::frequency_from_key_id (midi_event.note_on.note, graph->master_tune()))));
				handled = true;
			}
			break;
		}

		case MIDI::Event::NoteOff:
		{
			float velocity = midi_event.note_off.velocity / 127.0f;

			if (note_filter && (note_channel == 0 || note_channel == midi_event.note_off.channel + 1))
			{
				buffer.push (new VoiceEvent (t, midi_event.note_off.note, device._voice_ids[midi_event.note_off.note], VoiceEvent::Action::Drop));
				device._allocated_voice_id = device._voice_ids[midi_event.note_off.note];
				device._voice_ids[midi_event.note_off.note] = OmniVoice;
				handled = true;
			}

			if (note_velocity_filter && (note_velocity_channel == 0 || note_velocity_channel == midi_event.note_off.channel + 1))
			{
				buffer.push (new VoiceControllerEvent (t, device._allocated_voice_id, velocity));
				handled = true;
			}
			break;
		}

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
						controller_smoothing_setup (t, fvalue, 1_ms, smoothing, graph->sample_rate());
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
					controller_smoothing_setup (t, fvalue, 1_ms, smoothing, graph->sample_rate());
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
						channel_pressure_smoothing_setup (t, fvalue, 1_ms, smoothing, graph->sample_rate());
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
					// KeyPressure before NoteOn? A buggy device:
					if (device._voice_ids[key] == OmniVoice)
						break;
					float const fvalue = value / 127.0f;
					buffer.push (new VoiceControllerEvent (t, device._voice_ids[key], fvalue));
					handled = true;
					if (smoothing > 0)
						key_pressure_smoothing_setup (key, t, fvalue, 1_ms, smoothing, graph->sample_rate());
				}
			}
			break;
	}

	return handled;
}


void
Controller::generate_smoothing_events (EventBuffer& buffer, Graph* graph)
{
	if (smoothing == 0.0f)
		return;

	Timestamp const t = graph->timestamp();

	for (SmoothingParams* sp: { &_controller_smoother, &_channel_pressure_smoother })
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

	for (unsigned int key = 0; key < _key_pressure_smoother.size(); ++key)
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
	std::function<QDomElement (const char*)> create_element = [&element](const char* name) -> QDomElement {
		return element.ownerDocument().createElement (name);
	};

	std::function<const char* (bool)> bool_to_str = [](bool value) noexcept -> const char* {
		return value ? "true" : "false";
	};

	std::function<QString (int)> channel_to_str = [](int channel) noexcept -> QString {
		return channel == 0
			? "all"
			: QString ("%1").arg (channel);
	};

	element.setAttribute ("name", _name);
	element.setAttribute ("smoothing", smoothing.milliseconds());

	QDomElement note_filter_el = create_element ("note-filter");
	note_filter_el.setAttribute ("enabled", bool_to_str (note_filter));
	note_filter_el.setAttribute ("channel", channel_to_str (note_channel));

	QDomElement note_velocity_el = create_element ("note-velocity");
	note_velocity_el.setAttribute ("enabled", bool_to_str (note_velocity_filter));
	note_velocity_el.setAttribute ("channel", channel_to_str (note_velocity_channel));

	QDomElement note_pitch_el = create_element ("note-pitch");
	note_pitch_el.setAttribute ("enabled", bool_to_str (note_pitch_filter));
	note_pitch_el.setAttribute ("channel", channel_to_str (note_pitch_channel));

	QDomElement controller_filter_el = create_element ("controller-filter");
	controller_filter_el.setAttribute ("enabled", bool_to_str (controller_filter));
	controller_filter_el.setAttribute ("channel", channel_to_str (controller_channel));
	controller_filter_el.setAttribute ("controller-number", QString ("%1").arg (controller_number));
	controller_filter_el.setAttribute ("controller-invert", bool_to_str (controller_invert));

	QDomElement pitchbend_filter_el = create_element ("pitchbend-filter");
	pitchbend_filter_el.setAttribute ("enabled", bool_to_str (pitchbend_filter));
	pitchbend_filter_el.setAttribute ("channel", channel_to_str (pitchbend_channel));

	QDomElement channel_pressure_filter_el = create_element ("channel-pressure");
	channel_pressure_filter_el.setAttribute ("enabled", bool_to_str (channel_pressure_filter));
	channel_pressure_filter_el.setAttribute ("channel", channel_to_str (channel_pressure_channel));
	channel_pressure_filter_el.setAttribute ("invert", bool_to_str (channel_pressure_invert));

	QDomElement key_pressure_filter_el = create_element ("key-pressure");
	key_pressure_filter_el.setAttribute ("enabled", bool_to_str (key_pressure_filter));
	key_pressure_filter_el.setAttribute ("channel", channel_to_str (key_pressure_channel));
	key_pressure_filter_el.setAttribute ("invert", bool_to_str (key_pressure_invert));

	element.appendChild (note_filter_el);
	element.appendChild (note_velocity_el);
	element.appendChild (note_pitch_el);
	element.appendChild (controller_filter_el);
	element.appendChild (pitchbend_filter_el);
	element.appendChild (channel_pressure_filter_el);
	element.appendChild (key_pressure_filter_el);
}


void
Controller::load_state (QDomElement const& element)
{
	std::function<bool (QDomElement&)> is_enabled = [](QDomElement& e) -> bool {
		return e.attribute ("enabled") == "true";
	};

	std::function<bool (QDomElement&)> is_inverted = [](QDomElement& e) -> bool {
		return e.attribute ("invert") == "true";
	};

	std::function<int (QDomElement&)> get_channel = [](QDomElement& e) -> int {
		QString ch = e.attribute ("channel");
		return ch == "all" ? 0 : ch.toInt();
	};

	_name = element.attribute ("name", "<unnamed>");
	smoothing = 1_ms * element.attribute ("smoothing", "0").toInt();

	for (QDomElement& e: element)
	{
		if (e.tagName() == "note-filter")
		{
			note_filter = is_enabled (e);
			note_channel = get_channel (e);
		}
		else if (e.tagName() == "note-velocity")
		{
			note_velocity_filter = is_enabled (e);
			note_velocity_channel = get_channel (e);
		}
		else if (e.tagName() == "note-pitch")
		{
			note_pitch_filter = is_enabled (e);
			note_pitch_channel = get_channel (e);
		}
		else if (e.tagName() == "controller-filter")
		{
			controller_filter = is_enabled (e);
			controller_channel = get_channel (e);
			controller_number = e.attribute ("controller-number").toInt();
			controller_invert = is_inverted (e);
		}
		else if (e.tagName() == "pitchbend-filter")
		{
			pitchbend_filter = is_enabled (e);
			pitchbend_channel = get_channel (e);
		}
		else if (e.tagName() == "channel-pressure")
		{
			channel_pressure_filter = is_enabled (e);
			channel_pressure_channel = get_channel (e);
			channel_pressure_invert = is_inverted (e);
		}
		else if (e.tagName() == "key-pressure")
		{
			key_pressure_filter = is_enabled (e);
			key_pressure_channel = get_channel (e);
			key_pressure_invert = is_inverted (e);
		}
	}
}


void
Controller::reset_filters()
{
	note_filter = note_pitch_filter = note_velocity_filter = controller_filter =
		pitchbend_filter = channel_pressure_filter = key_pressure_filter = false;
}


void
Controller::controller_smoothing_setup (Timestamp t, float target, Seconds min_ms, Seconds max_ms, Frequency sample_rate)
{
	float dt = sample_rate * 1_us * (t - _controller_smoother.prev_timestamp).microseconds();
	_controller_smoother.target = target;
	_controller_smoother.smoother.set_samples (bound (dt, min_ms * sample_rate, max_ms * sample_rate));
	_controller_smoother.prev_timestamp = t;
}


void
Controller::channel_pressure_smoothing_setup (Timestamp t, float target, Seconds min_ms, Seconds max_ms, Frequency sample_rate)
{
	float dt = sample_rate * 1_us * (t - _channel_pressure_smoother.prev_timestamp).microseconds();
	_channel_pressure_smoother.target = target;
	_channel_pressure_smoother.smoother.set_samples (bound (dt, min_ms * sample_rate, max_ms * sample_rate));
	_channel_pressure_smoother.prev_timestamp = t;
}


void
Controller::key_pressure_smoothing_setup (unsigned int key, Timestamp t, float target, Seconds min_ms, Seconds max_ms, Frequency sample_rate)
{
	float dt = sample_rate * 1_us * (t - _key_pressure_smoother[key].prev_timestamp).microseconds();
	_key_pressure_smoother[key].target = target;
	_key_pressure_smoother[key].smoother.set_samples (bound (dt, min_ms * sample_rate, max_ms * sample_rate));
	_key_pressure_smoother[key].prev_timestamp = t;
}

} // namespace DevicesManager

} // namespace Haruhi

