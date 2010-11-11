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
#include <iterator>
#include <algorithm>
#include <memory>
#include <set>
#include <map>

// Qt:
#include <QtGui/QTabWidget>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QPixmap>
#include <QtGui/QToolTip>
#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3Grid>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/session.h>
#include <haruhi/core/audio_buffer.h>
#include <haruhi/core/event_buffer.h>
#include <haruhi/dsp/functions.h>
#include <haruhi/dsp/wavetable.h>
#include <haruhi/dsp/fourier_series_filler.h>
#include <haruhi/dsp/fft_filler.h>
#include <haruhi/dsp/noise.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/widgets/dial_control.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "mikuru.h"
#include "voice.h"
#include "part.h"
#include "widgets.h"
#include "general.h"
#include "common_filters.h"
#include "double_filter.h"


namespace Private = MikuruPrivate;


Mikuru::Mikuru (Haruhi::Session* session, std::string const& urn, std::string const& title, int id, QWidget* parent):
	Haruhi::Plugin (session, urn, title, id, parent),
	_patch (this),
	_general (0),
	_common_filters (0),
	_mix_L (64),
	_mix_R (64),
	_filter_buffer_L (64),
	_filter_buffer_R (64),
	_input_buffer_L (64),
	_input_buffer_R (64)
{
	// Widgets:

	_enabled = new QCheckBox ("Enabled (Note On)", this);
	QObject::connect (_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	_enabled->setChecked (true);

	QLabel* polyphony_label = new QLabel ("Voices number:", this);
	_current_polyphony = new QLabel ("0", this);

	_add_part = new QPushButton (Config::Icons16::add(), "Add part", this);
	QObject::connect (_add_part, SIGNAL (clicked()), this, SLOT (add_part()));
	QObject::connect (_add_part, SIGNAL (clicked()), this, SLOT (update_widgets()));

	_del_part = new QPushButton (Config::Icons16::remove(), "Remove part", this);
	QObject::connect (_del_part, SIGNAL (clicked()), this, SLOT (del_part()));
	QObject::connect (_del_part, SIGNAL (clicked()), this, SLOT (update_widgets()));

	_tabs_widget = new QTabWidget (this);
	_tabs_widget->setTabPosition (QTabWidget::North);
	_tabs_widget->setMovable (true);
	QObject::connect (_tabs_widget, SIGNAL (currentChanged (QWidget*)), this, SLOT (update_widgets()));

	_general = new Private::General (this, this);
	_common_filters = new Private::CommonFilters (this, this);

	_tabs_widget->addTab (_general, "Main && LFO/EG");
	_tabs_widget->addTab (_common_filters, "Filters");

	QVBoxLayout* layout = new QVBoxLayout (this, 0, Config::spacing);
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	QHBoxLayout* hor_layout = new QHBoxLayout (layout, Config::spacing);
	hor_layout->addWidget (_enabled);
	hor_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	hor_layout->addWidget (polyphony_label);
	hor_layout->addWidget (_current_polyphony);
	hor_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	hor_layout->addWidget (_add_part);
	hor_layout->addWidget (_del_part);

	layout->addWidget (_tabs_widget);

	// UI timer:
	_update_ui_timer = new QTimer (this);
	QObject::connect (_update_ui_timer, SIGNAL (timeout()), this, SLOT (update_ui()));
	_update_ui_timer->start (100); // 10 fps
}


Mikuru::~Mikuru()
{
	_update_ui_timer->stop();
}


void
Mikuru::registered()
{
	int sample_rate = graph()->sample_rate();

	// Smoothers (100ms):
	_audio_input_smoother_L.set_smoothing_samples (0.100f * sample_rate);
	_audio_input_smoother_R.set_smoothing_samples (0.100f * sample_rate);
	_master_volume_smoother_L.set_smoothing_samples (0.100f * sample_rate);
	_master_volume_smoother_R.set_smoothing_samples (0.100f * sample_rate);

	// Resize buffers:
	graph_updated();

	if (graph())
		graph()->lock();

	_audio_input_L = new Haruhi::Core::AudioPort (this, "L input", Haruhi::Core::Port::Input, 0, Haruhi::Core::Port::StandardAudio);
	_audio_input_R = new Haruhi::Core::AudioPort (this, "R input", Haruhi::Core::Port::Input, 0, Haruhi::Core::Port::StandardAudio);

	_audio_output_L = new Haruhi::Core::AudioPort (this, "L output", Haruhi::Core::Port::Output, 0, Haruhi::Core::Port::StandardAudio);
	_audio_output_R = new Haruhi::Core::AudioPort (this, "R output", Haruhi::Core::Port::Output, 0, Haruhi::Core::Port::StandardAudio);

	_port_keyboard = new Haruhi::Core::EventPort (this, "Keyboard", Haruhi::Core::Port::Input, 0, Haruhi::Core::Port::ControlKeyboard);
	_port_sustain = new Haruhi::Core::EventPort (this, "Sustain", Haruhi::Core::Port::Input, 0, Haruhi::Core::Port::ControlSustain);

	if (graph())
		graph()->unlock();

	load_config();

	enable();
}


void
Mikuru::unregistered()
{
	panic();

	// Stop synthesis threads:
	stop_threads();
	del_all_parts();

	_general->delete_ports();
	_common_filters->delete_ports();
	_general->delete_envelopes();

	delete _audio_input_L;
	delete _audio_input_R;

	delete _audio_output_L;
	delete _audio_output_R;

	delete _port_keyboard;
	delete _port_sustain;
}


std::string
Mikuru::name() const
{
	return "Mikuru";
}


void
Mikuru::process()
{
	clear_outputs();

	_parts_mutex.lock();

	// Order is important, first process events to create new Voices:
	process_voice_events();
	// Process envelopes for all voices (new too):
	process_envelopes();
	// Process controller events to apply envelope and other events:
	process_controller_events();
	// Then synthesize voices:
	process_voices();
	// Mixed output is in _mix_L and _mix_R.

	// Process audio input and filters:
	Private::Params::General gp = *_general->params();
	Private::Params::CommonFilters fp = *_common_filters->params();

	if (gp.enable_audio_input)
	{
		// Copy buffers:
		_input_buffer_L.fill (_audio_input_L->audio_buffer());
		_input_buffer_R.fill (_audio_input_R->audio_buffer());
		// Adjust volume:
		float v = std::pow (_general->params()->input_volume.to_f(), M_E);
		_audio_input_smoother_L.set_value (v);
		_audio_input_smoother_L.multiply (_input_buffer_L.begin(), _input_buffer_L.end());
		_audio_input_smoother_R.set_value (v);
		_audio_input_smoother_R.multiply (_input_buffer_R.begin(), _input_buffer_R.end());
	}

	if (gp.enable_audio_input && fp.route_audio_input)
	{
		_mix_L.mixin (&_input_buffer_L);
		_mix_R.mixin (&_input_buffer_R);
	}

	__brainfuck (">[-]>[-]<<[->+>+<<]>>[-<<+>>]<<", _mix_L, _mix_R);

	_common_filters->process_filters (_mix_L, _filter_buffer_L, *_audio_output_L->audio_buffer(),
									  _mix_R, _filter_buffer_R, *_audio_output_R->audio_buffer());

	if (gp.enable_audio_input && !fp.route_audio_input)
	{
		_audio_output_L->buffer()->mixin (&_input_buffer_L);
		_audio_output_R->buffer()->mixin (&_input_buffer_R);
	}

	// Master volume:
	Haruhi::Sample v = std::pow (general()->params()->volume.to_f(), M_E);
	_master_volume_smoother_L.set_value (v);
	_master_volume_smoother_L.multiply (_audio_output_L->audio_buffer()->begin(), _audio_output_L->audio_buffer()->end());
	_master_volume_smoother_R.set_value (v);
	_master_volume_smoother_R.multiply (_audio_output_R->audio_buffer()->begin(), _audio_output_R->audio_buffer()->end());

	for (Parts::iterator t = _parts.begin(); t != _parts.end(); ++t)
	{
		(*t)->voice_manager()->sweep();
		(*t)->voice_manager()->check_polyphony();
	}

	_parts_mutex.unlock();
}


void
Mikuru::panic()
{
	for (Parts::iterator t = _parts.begin(); t != _parts.end(); ++t)
		(*t)->voice_manager()->panic();
}


void
Mikuru::graph_updated()
{
	Unit::graph_updated();

	if (graph())
		graph()->lock();

	std::size_t s = graph()->buffer_size();
	_synth_threads_mutex.lock();
	for (MikuruPrivate::SynthThreads::iterator st = _synth_threads.begin(); st != _synth_threads.end(); ++st)
		(*st)->resize_buffers (s);
	_synth_threads_mutex.unlock();
	_mix_L.resize (s);
	_mix_R.resize (s);
	_filter_buffer_L.resize (s);
	_filter_buffer_R.resize (s);
	_input_buffer_L.resize (s);
	_input_buffer_R.resize (s);

	if (general() && general()->envelopes())
		general()->envelopes()->resize_buffers (s);

	if (graph())
		graph()->unlock();
}


void
Mikuru::notify (Haruhi::Core::Notification* notification)
{
	UpdateConfig* update_config = dynamic_cast<UpdateConfig*> (notification);
	if (update_config)
		if (update_config->sender() != this && graph())
			load_config();
}


void
Mikuru::set_unit_bay (Haruhi::UnitBay* unit_bay)
{
	UnitBayAware::set_unit_bay (unit_bay);
	_general->unit_bay_assigned();
	_common_filters->unit_bay_assigned();
	ensure_there_is_at_least_one_part();
}


void
Mikuru::save_state (QDomElement& element) const
{
	QDomElement patch = element.ownerDocument().createElement ("mikuru-patch");
	_patch.save_state (patch);
	element.appendChild (patch);
}


void
Mikuru::load_state (QDomElement const& element)
{
	disable();
	_common_filters->reset();
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull() && e.tagName() == "mikuru-patch")
			_patch.load_state (e);
	}
	ensure_there_is_at_least_one_part();
	_tabs_widget->showPage (_general);
	enable();
}


int
Mikuru::allocate_id (std::string const& group)
{
	int id = 1;
	for (; !ids (group).insert (id).second; ++id)
		;
	return id;
}


int
Mikuru::reserve_id (std::string const& group, int id)
{
	return ids (group).insert (id).second ? id : allocate_id (group);
}


void
Mikuru::free_id (std::string const& group, int id)
{
	ids (group).erase (id);
}


void
Mikuru::update_params()
{
	atomic (_param_enabled) = _enabled->isChecked();
}


void
Mikuru::update_widgets()
{
	_del_part->setEnabled (_parts.size() > 1 && dynamic_cast<Private::Part*> (_tabs_widget->currentPage()) != 0);
}


void
Mikuru::ensure_there_is_at_least_one_part()
{
	// Add first part:
	if (_parts.size() == 0)
		add_part();
}


Private::Part*
Mikuru::add_part()
{
	Private::Part* part = new Private::Part (this, this);
	_parts_mutex.lock();
	_parts.push_back (part);
	_parts_mutex.unlock();
	_tabs_widget->addTab (part, QString ("Part %1").arg (part->id()));
	_tabs_widget->showPage (part);
	// Notify envelopes about new Part:
	general()->envelopes()->notify_new_part (part);
	return part;
}


void
Mikuru::del_part()
{
	Private::Part* part = dynamic_cast<Private::Part*> (_tabs_widget->currentPage());
	if (part)
		del_part (part);
}


void
Mikuru::del_part (Private::Part* part)
{
	_parts_mutex.lock();
	_parts.remove (part);
	_parts_mutex.unlock();
	_tabs_widget->removePage (part);
	delete part;
}


void
Mikuru::del_all_parts()
{
	_parts_mutex.lock();
	while (_parts.size() > 0)
	{
		del_part (_parts.front());
		_parts_mutex.yield();
	}
	_parts_mutex.unlock();
}


void
Mikuru::update_ui()
{
	int i = 0;
	for (Mikuru::Parts::iterator t = _parts.begin(); t != _parts.end(); ++t)
		i += (*t)->voice_manager()->current_polyphony();
	_current_polyphony->setText (QString ("%1").arg (i));
}


void
Mikuru::save_config()
{
	Config::UnitConfiguration& uc = Config::unit_configuration (QString::fromStdString (urn()));
	uc.config_element().setAttribute ("threads-number", _general->threads_number());
	uc.save();
}


void
Mikuru::load_config()
{
	Config::UnitConfiguration& uc = Config::unit_configuration (QString::fromStdString (urn()));
	QString threads_number = uc.config_element().attribute ("threads-number");
	_general->set_threads_number (threads_number.isNull() ? 0 : threads_number.toInt());
}


void
Mikuru::sync_some_inputs()
{
	// Sync all inputs except keyboard and sustain (those
	// are manually synced in process_voice_events()).
	for (Haruhi::Core::Ports::const_iterator i = inputs().begin(); i != inputs().end(); ++i)
		if (*i != _port_keyboard && *i != _port_sustain)
			(*i)->sync();
}


void
Mikuru::stop_threads()
{
	_synth_threads_mutex.lock();
	for (Private::SynthThreads::iterator s = _synth_threads.begin(); s != _synth_threads.end(); ++s)
	{
		(*s)->exit();
		delete *s;
	}
	_synth_threads.clear();
	_synth_threads_mutex.unlock();
}


void
Mikuru::process_envelopes()
{
	_general->envelopes()->process();
}


void
Mikuru::process_voice_events()
{
	Haruhi::Core::EventBuffer* buffer;

	// Sustain:
	_port_sustain->sync();
	buffer = _port_sustain->event_buffer();
	// Find most recent ControllerEvent, throw the rest:
	for (Haruhi::Core::EventBuffer::EventsMultiset::reverse_iterator e = buffer->events().rbegin(); e != buffer->events().rend(); ++e)
	{
		if ((*e)->event_type() == Haruhi::Core::Event::ControllerEventType)
		{
			Haruhi::Core::ControllerEvent const* controller_event = static_cast<Haruhi::Core::ControllerEvent const*> (e->get());
			for (Parts::iterator t = _parts.begin(); t != _parts.end(); ++t)
				(*t)->voice_manager()->set_sustain (controller_event->value() >= 0.5);
			break;
		}
	}

	// Keyboard:
	_port_keyboard->sync();
	buffer = _port_keyboard->event_buffer();
	int enabled = atomic (_param_enabled);
	for (Haruhi::Core::EventBuffer::EventsMultiset::iterator e = buffer->events().begin(); e != buffer->events().end(); ++e)
	{
		if ((*e)->event_type() == Haruhi::Core::Event::VoiceEventType)
		{
			Haruhi::Core::VoiceEvent const* voice_event = static_cast<Haruhi::Core::VoiceEvent const*> (e->get());
			if (enabled || voice_event->type() == Haruhi::Core::VoiceEvent::Release || voice_event->type() == Haruhi::Core::VoiceEvent::Drop)
				for (Parts::iterator t = _parts.begin(); t != _parts.end(); ++t)
					(*t)->voice_manager()->voice_event (voice_event);
		}
	}

	// Force processing buffered events:
	for (Parts::iterator t = _parts.begin(); t != _parts.end(); ++t)
		(*t)->voice_manager()->process_buffered_events();
}


void
Mikuru::process_controller_events()
{
	sync_some_inputs();

	_general->process_events();
	_common_filters->process_events();
	for (Parts::iterator part = _parts.begin(); part != _parts.end(); ++part)
		(*part)->process_events();
}


void
Mikuru::process_voices()
{
	_synth_threads_mutex.lock();

	// Synthesize:
	for (Private::SynthThreads::iterator s = _synth_threads.begin(); s != _synth_threads.end(); ++s)
		(*s)->synthesize();
	// Wait for threads:
	for (Private::SynthThreads::iterator s = _synth_threads.begin(); s != _synth_threads.end(); ++s)
		(*s)->join_synthesized();

	_mix_L.clear();
	_mix_R.clear();
	// Mix threads' results:
	for (Private::SynthThreads::iterator s = _synth_threads.begin(); s != _synth_threads.end(); ++s)
	{
		_mix_L.mixin ((*s)->buffer1());
		_mix_R.mixin ((*s)->buffer2());
	}

	_synth_threads_mutex.unlock();
}


void
Mikuru::set_threads_number (int threads)
{
	if (threads == 0)
		threads = Haruhi::Haruhi::detected_cores();

	threads = std::min (16, std::max (1, threads));

	if (graph())
		graph()->lock();
	_synth_threads_mutex.lock();
	// Delete excess threads (over user-set limit):
	while (_synth_threads.size() > static_cast<unsigned int> (threads))
	{
		// Drop thread, move voices to remaining threads:
		Private::SynthThreads::iterator s = _synth_threads.begin();
		// Copy voices list:
		Private::Voices voices = (*s)->voices();
		(*s)->exit();
		delete *s;
		_synth_threads.erase (s);
		// Assign voices to new threads:
		for (Private::Voices::iterator v = voices.begin(); v != voices.end(); ++v)
			(*v)->set_thread (select_thread_for_new_voice());
		// Temporarily unlock graph in case synth thread is waiting for it:
		_synth_threads_mutex.unlock();
		if (graph())
			graph()->yield();
		_synth_threads_mutex.lock();
	}

	// Create new threads:
	while (_synth_threads.size() < static_cast<unsigned int> (threads))
	{
		Private::SynthThreads::iterator s = _synth_threads.insert (new Private::SynthThread (this)).first;
		(*s)->start();
		// Temporarily unlock graph in case synth thread is waiting for it:
		_synth_threads_mutex.unlock();
		if (graph())
			graph()->yield();
		_synth_threads_mutex.lock();
	}

	_synth_threads_mutex.unlock();
	if (graph())
		graph()->unlock();
}


Private::SynthThread*
Mikuru::select_thread_for_new_voice()
{
	_synth_threads_mutex.lock();
	if (_synth_threads.empty())
	{
		_synth_threads_mutex.unlock();
		return 0;
	}

	Private::SynthThreads::iterator f = _synth_threads.begin();
	Private::Voices::size_type min = (*f)->voices().size();
	// Find thread with least voices:
	for (Private::SynthThreads::iterator s = _synth_threads.begin(); s != _synth_threads.end(); ++s)
	{
		if ((*s)->voices().size() < min)
		{
			f = s;
			min = (*s)->voices().size();
		}
	}
	_synth_threads_mutex.unlock();
	return *f;
}


MikuruFactory::MikuruFactory():
	Haruhi::PluginFactory()
{
	_information["haruhi:urn"] = "urn://haruhi.mulabs.org/synth/mikuru/1";
	_information["haruhi:presets.directory"] = "mikuru-1";
	_information["haruhi:title"] = "Mikuru";
	_information["haruhi:author.name"] = "Michał <mcv> Gawron";
	_information["haruhi:author.contact.0"] = "mailto:michal@gawron.name";
	_information["haruhi:author.contact.1"] = "xmpp:mcv@jabber.org";
	_information["haruhi:license"] = "GPL-3.0";
}


Haruhi::Plugin*
MikuruFactory::create_plugin (Haruhi::Session* session, int id, QWidget* parent)
{
	return new Mikuru (session, _information["haruhi:urn"], _information["haruhi:title"], id, parent);
}


void
MikuruFactory::destroy_plugin (Haruhi::Plugin* plugin)
{
	delete plugin;
}


MikuruFactory::InformationMap const&
MikuruFactory::information() const
{
	return _information;
}

