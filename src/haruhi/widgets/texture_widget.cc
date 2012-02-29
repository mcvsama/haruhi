/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <QtGui/QSizePolicy>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "texture_widget.h"


namespace Haruhi {

TextureWidget::TextureWidget (Filling filling, QWidget* parent, const char* name):
	QWidget (parent, name),
	_filling (filling)
{
}


void
TextureWidget::moveEvent (QMoveEvent*)
{
	update();
}


void
TextureWidget::paintEvent (QPaintEvent* paint_event)
{
	_double_buffer.resize (paint_event->rect().size());
	QPainter b (&_double_buffer);
	QColor dark = palette().color (QPalette::Mid);
	QColor light = palette().color (QPalette::Light);

	switch (_filling)
	{
		case Filling::Solid:
			b.fillRect (rect(), backgroundColor());
			break;

		case Filling::Dotted:
			b.fillRect (rect(), backgroundColor());
			for (int x = 0; x < _double_buffer.size().width(); x += 10)
			{
				for (int y = 0; y < _double_buffer.size().height(); y += 10)
				{
					QPoint p (((y % 20 == 10) ? x : x + 5) + 3, y + 3);
					b.setPen (QPen (dark, 1));
					b.drawPoint (p);
					b.setPen (QPen (light, 1));
					b.drawPoint (p + QPoint (1, 1));
				}
			}
			break;
	}

	QPainter (this).drawPixmap (paint_event->rect().topLeft(), _double_buffer, _double_buffer.rect());
}

} // namespace Haruhi

