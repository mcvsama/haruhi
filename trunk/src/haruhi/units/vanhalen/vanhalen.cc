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

// Qt:
#include <qwidget.h>

// Local:
#include <core/event_buffer.h>
#include <core/audio_buffer.h>

#include "vanhalen.h"


VanHalen::VanHalen (Haruhi::UnitFactory* factory, Haruhi::Session* session, std::string const& urn, std::string const& title, int id, QWidget* parent):
	Haruhi::Unit (factory, session, urn, title, id, parent)
{
	register_unit();

	_input = new Core::EventPort (this, "Input", Core::Port::Input);
	_output = new Core::EventPort (this, "Output", Core::Port::Output);

	_audio_input_1 = new Core::AudioPort (this, "Audio L", Core::Port::Input);
	_audio_input_2 = new Core::AudioPort (this, "Audio R", Core::Port::Input);

	_audio_output_1 = new Core::AudioPort (this, "Audio L", Core::Port::Output);
	_audio_output_2 = new Core::AudioPort (this, "Audio R", Core::Port::Output);

	enable();
}


VanHalen::~VanHalen()
{
	panic();
	disable();

	delete _input;
	delete _output;
	delete _audio_input_1;
	delete _audio_input_2;
	delete _audio_output_1;
	delete _audio_output_2;

	unregister_unit();
}


std::string
VanHalen::name() const
{
	return "VanHalen";
}


void
VanHalen::process()
{
	sync_inputs();
	clear_outputs();

	Core::EventBuffer* buffer = _input->event_buffer();

	// Keyboard events:
	for (Core::EventBuffer::EventsMultiset::iterator e = buffer->events().begin(); e != buffer->events().end(); ++e)
	{
		Core::VoiceEvent const* voice_event = dynamic_cast<Core::VoiceEvent const*> (*e);
		if (voice_event)
		{
			if (voice_event->type() == Core::VoiceEvent::Create)
			{
				// TODO
			}
			else if (voice_event->type() == Core::VoiceEvent::Release || voice_event->type() == Core::VoiceEvent::Drop)
			{
				// TODO
			}
		}
	}

	// Audio quantization effect:
	Core::AudioBuffer* i1 = _audio_input_1->audio_buffer();
	Core::AudioBuffer* i2 = _audio_input_1->audio_buffer();
	Core::AudioBuffer* o1 = _audio_output_1->audio_buffer();
	Core::AudioBuffer* o2 = _audio_output_1->audio_buffer();
	int keep_bits = 8;
	for (unsigned int i = 0; i < i1->size(); ++i)
	{
		(*o1)[i] = (static_cast<int> (32768.0 * (*i1)[i]) & (-1 << (16 - keep_bits))) / 32768.0f;
		(*o2)[i] = (static_cast<int> (32768.0 * (*i2)[i]) & (-1 << (16 - keep_bits))) / 32768.0f;
	}
}


void
VanHalen::panic()
{
}


VanHalenFactory::VanHalenFactory():
	Haruhi::UnitFactory()
{
	_information["haruhi:urn"] = "urn://haruhi.mulabs.org/synth/vanhalen/1";
	_information["haruhi:presets.directory"] = "vanhalen-1";
	_information["haruhi:title"] = "VanHalen";
	_information["haruhi:author.name"] = "Michał <mcv> Gawron";
	_information["haruhi:author.contact.0"] = "mailto:michal@gawron.name";
	_information["haruhi:author.contact.1"] = "xmpp:mcv@jabber.org";
	_information["haruhi:license"] = "GPL-3.0";
}


Haruhi::Unit*
VanHalenFactory::create_unit (Haruhi::Session* session, int id, QWidget* parent)
{
	return new VanHalen (this, session, _information["haruhi:urn"], _information["haruhi:title"], id, parent);
}


void
VanHalenFactory::destroy_unit (Haruhi::Unit* unit)
{
	delete unit;
}


VanHalenFactory::InformationMap const&
VanHalenFactory::information() const
{
	return _information;
}

