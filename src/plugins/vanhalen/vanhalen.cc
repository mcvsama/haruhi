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
#include <memory>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QLayout>

// Local:
#include <haruhi/config/system.h>
#include <haruhi/graph/event_buffer.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/delay_line.h>

#include "vanhalen.h"


VanHalen::VanHalen (std::string const& urn, std::string const& title, int id, QWidget* parent):
	Haruhi::Plugin (urn, title, id, parent),
	_buf1 (64),
	_buf2 (64),
	_delay1 (16, 100000, 64),
	_delay2 (16, 100000, 64),
	_comb_index (0, 1000, 0, 0, 1000),
	_comb_alpha (-1000, 1000, 0, 0, 1000)
{
	_input = new Haruhi::EventPort (this, "Input", Haruhi::Port::Input);
	_output = new Haruhi::EventPort (this, "Output", Haruhi::Port::Output);

	_audio_input_1 = new Haruhi::AudioPort (this, "Audio L", Haruhi::Port::Input);
	_audio_input_2 = new Haruhi::AudioPort (this, "Audio R", Haruhi::Port::Input);

	_audio_output_1 = new Haruhi::AudioPort (this, "Audio L", Haruhi::Port::Output);
	_audio_output_2 = new Haruhi::AudioPort (this, "Audio R", Haruhi::Port::Output);

	_knob_comb_index = new Haruhi::Knob (this, 0, &_comb_index, "Index", 0, 1000, 1, 0);
	_knob_comb_alpha = new Haruhi::Knob (this, 0, &_comb_alpha, "Alpha", -1.0, 1.0, 10, 2);

	QHBoxLayout* layout = new QHBoxLayout (this, Config::spacing());
	layout->addWidget (_knob_comb_index);
	layout->addWidget (_knob_comb_alpha);
}


VanHalen::~VanHalen()
{
	delete _input;
	delete _output;
	delete _audio_input_1;
	delete _audio_input_2;
	delete _audio_output_1;
	delete _audio_output_2;
}


void
VanHalen::registered()
{
	graph_updated();

	enable();
}


void
VanHalen::unregistered()
{
	panic();
}


void
VanHalen::process()
{
	sync_inputs();
	clear_outputs();

#if 0
	Haruhi::EventBuffer* buffer = _input->event_buffer();

	// Keyboard events:
	for (Haruhi::EventBuffer::EventsMultiset::iterator e = buffer->events().begin(); e != buffer->events().end(); ++e)
	{
		Haruhi::VoiceEvent const* voice_event = dynamic_cast<Haruhi::VoiceEvent const*> (*e);
		if (voice_event)
		{
			if (voice_event->type() == Haruhi::VoiceEvent::Create)
			{
				// TODO
			}
			else if (voice_event->type() == Haruhi::VoiceEvent::Release || voice_event->type() == Haruhi::VoiceEvent::Drop)
			{
				// TODO
			}
		}
	}
#endif

	Haruhi::AudioBuffer* i1 = _audio_input_1->audio_buffer();
	Haruhi::AudioBuffer* i2 = _audio_input_2->audio_buffer();
	Haruhi::AudioBuffer* o1 = _audio_output_1->audio_buffer();
	Haruhi::AudioBuffer* o2 = _audio_output_2->audio_buffer();

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


void
VanHalen::graph_updated()
{
	Unit::graph_updated();

	_buf1.resize (graph()->buffer_size());
	_buf2.resize (graph()->buffer_size());
	_delay1.set_size (graph()->buffer_size());
	_delay2.set_size (graph()->buffer_size());
}


VanHalenFactory::VanHalenFactory():
	Haruhi::PluginFactory()
{
}


Haruhi::Plugin*
VanHalenFactory::create_plugin (int id, QWidget* parent)
{
	return new VanHalen (urn(), title(), id, parent);
}


void
VanHalenFactory::destroy_plugin (Haruhi::Plugin* plugin)
{
	delete plugin;
}

