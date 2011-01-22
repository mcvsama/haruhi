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

#ifndef HARUHI__UNITS__MIKURU__EFFECTS__DELAY_H__INCLUDED
#define HARUHI__UNITS__MIKURU__EFFECTS__DELAY_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>

// Local:
#include "effect.h"


class Mikuru;

namespace MikuruPrivate {

class Delay: public Effect
{
  public:
	Delay (int id, Mikuru* mikuru, QWidget* parent);

	~Delay();

	int
	id() const { return _id; }

  private:
	Mikuru*	_mikuru;
	int		_id;
};

} // namespace MikuruPrivate

#endif

