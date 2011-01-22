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

#ifndef HARUHI__UNITS__MIKURU__EFFECTS__WAVESHAPER_H__INCLUDED
#define HARUHI__UNITS__MIKURU__EFFECTS__WAVESHAPER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Qt:
#include <QtGui/QWidget>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "effect.h"


class Mikuru;

namespace MikuruPrivate {

class Waveshaper: public Effect
{
  public:
	Waveshaper (int id, Mikuru* mikuru, QWidget* parent);

	~Waveshaper();

	int
	id() const { return _id; }

  private:
	/**
	 * Gloubi-boulga waveshaper
	 * <http://musicdsp.org/showArchiveComment.php?ArchiveID=86>
	 */
	Sample
	waveshape_gloubi_boulga (Sample input)
	{
		const float x = input * 0.686306f;
		const float a = 1.0f + expf (sqrtf (fabsf (x)) * -0.75f);
		return (expf (x) - expf (-x * a)) / (expf (x) + expf (-x));
	}

	/**
	 * Simpler implementation of Gloubi-boulga waveshaper
	 * x - 0.15 * x^2 - 0.15 * x^3
	 */
	Sample
	waveshape_gloubi_boulga_simple (Sample x)
	{
		return x - 0.15f * x * x - 0.15f * x * x * x;
	}

	/**
	 * Simple cubic waveshaper
	 * <http://musicdsp.org/showArchiveComment.php?ArchiveID=114>
	 */
	Sample
	waveshape_power3 (Sample x)
	{
		return 1.5f * x - 0.5f * x * x * x;
	}

  private:
	Mikuru*	_mikuru;
	int		_id;
};

} // namespace MikuruPrivate

#endif

