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

#ifndef HARUHI__WIDGETS__TEXTURE_WIDGET_H__INCLUDED
#define HARUHI__WIDGETS__TEXTURE_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

class TextureWidget: public QWidget
{
  public:
	enum class Filling { Solid, Dotted };

  public:
	TextureWidget (Filling filling, QWidget* parent, const char* name = 0);

  protected:
	void
	moveEvent (QMoveEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

  private:
	Filling	_filling;
	QPixmap	_double_buffer;
};

} // namespace Haruhi

#endif

