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
#include <memory>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QLayout>

// Local:
#include <haruhi/config/system.h>
#include <haruhi/core/event_buffer.h>
#include <haruhi/core/audio_buffer.h>
#include <haruhi/dsp/delay_line.h>
#include <haruhi/session.h>

#include "vanhalen.h"


VanHalen::VanHalen (Haruhi::UnitFactory* factory, Haruhi::Session* session, std::string const& urn, std::string const& title, int id, QWidget* parent):
	Haruhi::Unit (factory, session, urn, title, id, parent),
	_buf1 (session->graph()->buffer_size()),
	_buf2 (session->graph()->buffer_size()),
	_delay1 (16, 100000, session->graph()->buffer_size()),
	_delay2 (16, 100000, session->graph()->buffer_size()),
	_comb_index (0, 0, 1000, 0),
	_comb_alpha (0, -1000, 1000, 0)
{
	register_unit();

	_input = new Haruhi::Core::EventPort (this, "Input", Haruhi::Core::Port::Input);
	_output = new Haruhi::Core::EventPort (this, "Output", Haruhi::Core::Port::Output);

	_audio_input_1 = new Haruhi::Core::AudioPort (this, "Audio L", Haruhi::Core::Port::Input);
	_audio_input_2 = new Haruhi::Core::AudioPort (this, "Audio R", Haruhi::Core::Port::Input);

	_audio_output_1 = new Haruhi::Core::AudioPort (this, "Audio L", Haruhi::Core::Port::Output);
	_audio_output_2 = new Haruhi::Core::AudioPort (this, "Audio R", Haruhi::Core::Port::Output);

	_proxy_comb_index = new Haruhi::ControllerProxy (0, &_comb_index);
	_proxy_comb_alpha = new Haruhi::ControllerProxy (0, &_comb_alpha);

	_knob_comb_index = new Haruhi::Knob (this, _proxy_comb_index, "Index", 0, 1000, 1, 0);
	_knob_comb_alpha = new Haruhi::Knob (this, _proxy_comb_alpha, "Alpha", -1.0, 1.0, 10, 2);

	QHBoxLayout* layout = new QHBoxLayout (this, Config::spacing);
	layout->addWidget (_knob_comb_index);
	layout->addWidget (_knob_comb_alpha);

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

#if 0
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
#endif

	Haruhi::Core::AudioBuffer* i1 = _audio_input_1->audio_buffer();
	Haruhi::Core::AudioBuffer* i2 = _audio_input_2->audio_buffer();
	Haruhi::Core::AudioBuffer* o1 = _audio_output_1->audio_buffer();
	Haruhi::Core::AudioBuffer* o2 = _audio_output_2->audio_buffer();

#if 0
	// Audio quantization effect:
	int keep_bits = 8;
	for (unsigned int i = 0; i < i1->size(); ++i)
	{
		(*o1)[i] = (static_cast<int> (32768.0 * (*i1)[i]) & (-1 << (16 - keep_bits))) / 32768.0f;
		(*o2)[i] = (static_cast<int> (32768.0 * (*i2)[i]) & (-1 << (16 - keep_bits))) / 32768.0f;
	}
#endif

#if 1
	// Feed-forward:
	_delay1.set_delay (_comb_index.get());
	_delay2.set_delay (_comb_index.get());

	o1->fill (i1);
	o2->fill (i2);

	_delay1.write (i1->begin());
	_delay2.write (i2->begin());

	_delay1.read (_buf1.begin());
	_delay2.read (_buf2.begin());

	for (unsigned int i = 0; i < _buf1.size(); ++i)
	{
		(*o1)[i] += _buf1[i] * 10.0f * _comb_alpha.get() / 1000.0f;
		(*o2)[i] += _buf2[i] * 10.0f * _comb_alpha.get() / 1000.0f;
	}
#else
	// Feed-back:
	_delay1.set_delay (_comb_index.get());
	_delay2.set_delay (_comb_index.get());

	_delay1.read (o1->begin());
	_delay2.read (o2->begin());

	for (unsigned int i = 0; i < _buf1.size(); ++i)
	{
		(*o1)[i] *= 1.0f * _comb_alpha / 1000.0f;
		(*o2)[i] *= 1.0f * _comb_alpha / 1000.0f;
	}

	o1->mixin (i1);
	o2->mixin (i2);

	_delay1.write (o1->begin());
	_delay2.write (o2->begin());
#endif
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

