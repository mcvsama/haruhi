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

#ifndef HARUHI__UNITS__MIKURU__EFFECTS__EFFECT_H__INCLUDED
#define HARUHI__UNITS__MIKURU__EFFECTS__EFFECT_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QPushButton>

// Haruhi:
#include <haruhi/graph/audio_buffer.h>

// Local:
#include "../params.h"
#include "effect.h"


class Mikuru;

namespace MikuruPrivate {

class Effect: public QWidget
{
  public:
	Effect (QWidget* parent);

	virtual ~Effect() { }

	virtual int
	id() const = 0;

	virtual void
	process (Haruhi::AudioBuffer* data, unsigned int channel) = 0;

  public slots:
	/**
	 * Loads widgets values from Params struct.
	 */
	virtual void
	load_params();

	/**
	 * Updates Params structure from widgets.
	 */
	virtual void
	update_params();

  protected:
	virtual Params::Effect&
	effect_params() = 0;

	QWidget*
	parent_widget() const;

  private:
	QWidget*		_effect_panel;
	QPushButton*	_enabled_button;
};

} // namespace MikuruPrivate

#endif
