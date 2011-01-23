/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
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
#include <QtGui/QLayout>
#include <QtGui/QToolTip>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "mikuru.h"
#include "part_effects.h"
#include "effects/delay.h"
#include "effects/reverb.h"
#include "effects/waveshaper.h"


namespace MikuruPrivate {

PartEffects::PartEffects (Part* part, Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_part (part)
{
	_effects_menu = new QMenu (this);
	_effects_menu->addAction ("Delay", this, SLOT (add_delay()));
	_effects_menu->addAction ("Reverb (Freeverb)", this, SLOT (add_reverb()));
	_effects_menu->addAction ("Waveshaper", this, SLOT (add_waveshaper()));

	_add_effect = new QPushButton (Resources::Icons16::add(), "Add effect", this);
	_add_effect->setMenu (_effects_menu);

	_remove_effect = new QPushButton (Resources::Icons16::remove(), "", this);
	QObject::connect (_remove_effect, SIGNAL (clicked()), this, SLOT (destroy_effect()));
	QToolTip::add (_remove_effect, "Remove selected effect");

	_placeholder = new QLabel ("Add effects with \"Add effect\" button.", this);
	_placeholder->setAlignment (Qt::AlignCenter);

	_tabs = new QTabWidget (this);
	_tabs->setMovable (true);

	_stack = new QStackedWidget (this);
	_stack->addWidget (_tabs);
	_stack->addWidget (_placeholder);

	// Layout:

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::Spacing);
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (_add_effect);
	buttons_layout->addWidget (_remove_effect);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::Margin);
	layout->setSpacing (Config::Spacing);
	layout->addLayout (buttons_layout);
	layout->addWidget (_stack);

	update_widgets();
}


void
PartEffects::process (Haruhi::AudioBuffer* buffer, unsigned int channel)
{
	_effects_mutex.lock();
	// TODO change relative filter position when tab is moved.
	for (EffectsList::iterator e = _effects.begin(); e != _effects.end(); ++e)
		(*e)->process (buffer, channel);
	_effects_mutex.unlock();
}


Delay*
PartEffects::add_delay (int id)
{
	Delay* effect = new Delay (id, _mikuru, this);
	_effects_mutex.lock();
	_effects.push_back (effect);
	_effects_mutex.unlock();
	_tabs->addTab (effect, QString ("Delay %1").arg (effect->id()));
	_tabs->showPage (effect);
	update_widgets();
	return effect;
}


Reverb*
PartEffects::add_reverb (int id)
{
	Reverb* effect = new Reverb (id, _mikuru, this);
	_effects_mutex.lock();
	_effects.push_back (effect);
	_effects_mutex.unlock();
	_tabs->addTab (effect, QString ("Reverb %1").arg (effect->id()));
	_tabs->showPage (effect);
	update_widgets();
	return effect;
}


Waveshaper*
PartEffects::add_waveshaper (int id)
{
	Waveshaper* effect = new Waveshaper (id, _mikuru, this);
	_effects_mutex.lock();
	_effects.push_back (effect);
	_effects_mutex.unlock();
	_tabs->addTab (effect, QString ("Waveshaper %1").arg (effect->id()));
	_tabs->showPage (effect);
	update_widgets();
	return effect;
}


void
PartEffects::destroy_effect()
{
	QWidget* tab = _tabs->currentPage();
	if (!tab)
		return;
	Effect* effect = dynamic_cast<Effect*> (tab);
	if (!effect)
		return;
	_effects_mutex.lock();
	_effects.remove (effect);
	_effects_mutex.unlock();
	_tabs->removePage (effect);
	delete effect;
	update_widgets();
}


void
PartEffects::destroy_effect (Effect* effect)
{
	_effects_mutex.lock();
	_effects.remove (effect);
	_effects_mutex.unlock();
	_tabs->removePage (effect);
	delete effect;
	update_widgets();
}


void
PartEffects::destroy_all_effects()
{
	_effects_mutex.lock();
	while (_effects.size() > 0)
	{
		destroy_effect (_effects.front());
		_effects_mutex.yield();
	}
	_effects_mutex.unlock();
}


void
PartEffects::show_first()
{
	if (!_effects.empty())
		_tabs->showPage (_effects.front());
}


void
PartEffects::update_widgets()
{
	if (_tabs->count())
		_stack->setCurrentWidget (_tabs);
	else
		_stack->setCurrentWidget (_placeholder);
	_remove_effect->setEnabled (_tabs->currentPage());
	_tabs->setMargin (Config::Spacing - 1);
}

} // namespace MikuruPrivate

