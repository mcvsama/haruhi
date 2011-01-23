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

#ifndef HARUHI__UNITS__MIKURU__PART_EFFECTS_H__INCLUDED
#define HARUHI__UNITS__MIKURU__PART_EFFECTS_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QMenu>
#include <QtGui/QStackedWidget>
#include <QtGui/QLabel>

// Haruhi:
#include <haruhi/utility/mutex.h>

// Local:
#include "effects/effect.h"
#include "effects/delay.h"
#include "effects/reverb.h"
#include "effects/waveshaper.h"


class Mikuru;

namespace MikuruPrivate {

class Part;

class PartEffects: public QWidget
{
	Q_OBJECT

  public:
	typedef std::list<Effect*> EffectsList;

  public:
	PartEffects (Part* part, Mikuru* mikuru, QWidget* parent);

	EffectsList&
	effects() { return _effects; }

	Mutex&
	effects_mutex() { return _effects_mutex; }

	/**
	 * Returns tab-poisition of given effect.
	 */
	int
	effect_tab_position (Effect* effect) const { return _tabs->indexOf (effect); }

	void
	process (Haruhi::AudioBuffer* buffer, unsigned int channel);

  public slots:
	Delay*
	add_delay (int id = 0);

	Reverb*
	add_reverb (int id = 0);

	Waveshaper*
	add_waveshaper (int id = 0);

	void
	destroy_effect();

	void
	destroy_effect (Effect*);

	void
	destroy_all_effects();

	void
	show_first();

  private:
	void
	update_widgets();

  private:
	Mikuru*			_mikuru;
	Part*			_part;
	EffectsList		_effects;
	Mutex			_effects_mutex;

	QPushButton*	_add_effect;
	QPushButton*	_remove_effect;
	QMenu*			_effects_menu;
	QTabWidget*		_tabs;
	QStackedWidget*	_stack;
	QLabel*			_placeholder;
};

} // namespace MikuruPrivate

#endif

