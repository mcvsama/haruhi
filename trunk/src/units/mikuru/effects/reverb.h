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

#ifndef HARUHI__UNITS__MIKURU__EFFECTS__REVERB_H__INCLUDED
#define HARUHI__UNITS__MIKURU__EFFECTS__REVERB_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>

// Local:
#include "effect.h"


class Mikuru;

namespace MikuruPrivate {

class Reverb: public Effect
{
  public:
	Reverb (int id, Mikuru* mikuru, QWidget* parent);

	~Reverb();

	int
	id() const { return _id; }

	void
	process (Haruhi::AudioBuffer* buffer, unsigned int channel);

  private:
	Mikuru*	_mikuru;
	int		_id;
};

} // namespace MikuruPrivate

#endif

