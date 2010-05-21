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
#include <algorithm>
#include <stdint.h>
#include <cmath>
#include <set>

// Qt:
#include <QtGui/QGroupBox>
#include <QtGui/QToolTip>
#include <Qt3Support/Q3GroupBox>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/utility/memory.h>

// Local:
#include "mikuru.h"
#include "common_filters.h"
#include "voice.h"
#include "voice_manager.h"
#include "part.h"
#include "envelopes.h"
#include "envelopes_adsr.h"
#include "envelopes_lfo.h"


namespace MikuruPrivate {

Envelope::Envelope (QWidget* parent):
	QWidget (parent)
{
}


Envelopes::Envelopes (Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru)
{
	_stack = new QStackedWidget (this);

	_placeholder = new QLabel ("Add envelopes/LFOs with \"Add\" buttons.", _stack);
	_placeholder->setAlignment (Qt::AlignCenter);
	_tabs = new QTabWidget (_stack);
	_tabs->setMovable (true);

	_stack->addWidget (_tabs);
	_stack->addWidget (_placeholder);

	_add_adsr = new QPushButton (Config::Icons16::adsr(), "Add ADSR", this);
	QObject::connect (_add_adsr, SIGNAL (clicked()), this, SLOT (add_adsr()));
	_add_envelope = new QPushButton (Config::Icons16::adsr(), "Add envelope", this);
	QObject::connect (_add_envelope, SIGNAL (clicked()), this, SLOT (add_envelope()));
	_add_lfo = new QPushButton (Config::Icons16::lfo(), "Add LFO", this);
	QObject::connect (_add_lfo, SIGNAL (clicked()), this, SLOT (add_lfo()));

	_remove_envelope = new QPushButton (Config::Icons16::remove(), "Remove", this);
	QObject::connect (_remove_envelope, SIGNAL (clicked()), this, SLOT (destroy_envelope()));

	// Layouts:

	QVBoxLayout* v1 = new QVBoxLayout (this, 0, Config::spacing);
	QHBoxLayout* h1 = new QHBoxLayout (v1, Config::spacing);
	h1->addWidget (_add_adsr);
	h1->addWidget (_add_envelope);
	h1->addWidget (_add_lfo);
	h1->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	h1->addWidget (_remove_envelope);
	v1->addWidget (_stack);

	update_widgets();
}


Envelopes::~Envelopes()
{
	// Delete all envelopes:
	QWidget* page;
	while ((page = _tabs->currentPage()))
		_tabs->removePage (page);
	_envelopes_mutex.lock();
	std::for_each (_envelopes.begin(), _envelopes.end(), delete_operator<Envelope*>);
	_envelopes_mutex.unlock();
}


void
Envelopes::process()
{
	_envelopes_mutex.lock();
	for (EnvelopesList::iterator e = _envelopes.begin(); e != _envelopes.end(); ++e)
		(*e)->process();
	_envelopes_mutex.unlock();
}


void
Envelopes::resize_buffers (std::size_t size)
{
	_envelopes_mutex.lock();
	for (EnvelopesList::iterator e = _envelopes.begin(); e != _envelopes.end(); ++e)
		(*e)->resize_buffers (size);
	_envelopes_mutex.unlock();
}


void
Envelopes::notify_voice_created (VoiceManager* voice_manager, Voice* voice)
{
	_envelopes_mutex.lock();
	for (EnvelopesList::iterator e = _envelopes.begin(); e != _envelopes.end(); ++e)
		(*e)->voice_created (voice_manager, voice);
	_envelopes_mutex.unlock();
}


void
Envelopes::notify_voice_released (VoiceManager* voice_manager, Voice* voice)
{
	_envelopes_mutex.lock();
	for (EnvelopesList::iterator e = _envelopes.begin(); e != _envelopes.end(); ++e)
		(*e)->voice_released (voice_manager, voice);
	_envelopes_mutex.unlock();
}


void
Envelopes::notify_voice_dropped (VoiceManager* voice_manager, Voice* voice)
{
	_envelopes_mutex.lock();
	for (EnvelopesList::iterator e = _envelopes.begin(); e != _envelopes.end(); ++e)
		(*e)->voice_dropped (voice_manager, voice);
	_envelopes_mutex.unlock();
}


void
Envelopes::notify_new_part (Part* part)
{
	_envelopes_mutex.lock();
	for (EnvelopesList::iterator e = _envelopes.begin(); e != _envelopes.end(); ++e)
		(*e)->new_part (part);
	_envelopes_mutex.unlock();
}


void
Envelopes::update_widgets()
{
	if (_tabs->count())
		_stack->setCurrentWidget (_tabs);
	else
		_stack->setCurrentWidget (_placeholder);
	_remove_envelope->setEnabled (_tabs->currentPage());
	_tabs->setMargin (Config::spacing - 1);
}


ADSR*
Envelopes::add_adsr (int id)
{
	ADSR* envelope = new ADSR (id, _mikuru, this);
	_envelopes_mutex.lock();
	_envelopes.push_back (envelope);
	_envelopes_mutex.unlock();
	_tabs->addTab (envelope, QString ("ADSR %1").arg (envelope->id()));
	_tabs->showPage (envelope);
	update_widgets();
	return envelope;
}


LFO*
Envelopes::add_lfo (int id)
{
	LFO* envelope = new LFO (id, _mikuru, this);
	_envelopes_mutex.lock();
	_envelopes.push_back (envelope);
	_envelopes_mutex.unlock();
	_tabs->addTab (envelope, QString ("LFO %1").arg (envelope->id()));
	_tabs->showPage (envelope);
	update_widgets();
	return envelope;
}


void
Envelopes::destroy_envelope()
{
	QWidget* tab = _tabs->currentPage();
	if (tab)
	{
		Envelope* envelope = dynamic_cast<Envelope*> (tab);
		if (envelope)
		{
			_envelopes_mutex.lock();
			_envelopes.remove (envelope);
			_envelopes_mutex.unlock();
			_tabs->removePage (envelope);
			delete envelope;
			update_widgets();
		}
	}
}


void
Envelopes::destroy_envelope (Envelope* envelope)
{
	_envelopes_mutex.lock();
	_envelopes.remove (envelope);
	_envelopes_mutex.unlock();
	_tabs->removePage (envelope);
	delete envelope;
	update_widgets();
}


void
Envelopes::destroy_all_envelopes()
{
	_envelopes_mutex.lock();
	while (_envelopes.size() > 0)
	{
		destroy_envelope (_envelopes.front());
		_envelopes_mutex.yield();
	}
	_envelopes_mutex.unlock();
}


void
Envelopes::show_first()
{
	if (!_envelopes.empty())
		_tabs->showPage (_envelopes.front());
}

} // namespace MikuruPrivate

