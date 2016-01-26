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

// Standard:
#include <cstddef>

// Qt:
#include <QSizePolicy>
#include <QPainter>
#include <QPaintEvent>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>

// Local:
#include "texture_widget.h"


namespace Haruhi {

using namespace ScreenLiterals;


TextureWidget::TextureWidget (Filling filling, QWidget* parent):
	QWidget (parent),
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
	_double_buffer = QPixmap (paint_event->rect().size());
	QPainter b (&_double_buffer);
	QColor dark = palette().color (QPalette::Mid);
	QColor light = palette().color (QPalette::Light);

	switch (_filling)
	{
		case Filling::Solid:
			b.fillRect (rect(), palette().color (QPalette::Window));
			break;

		case Filling::Dotted:
		{
			b.fillRect (rect(), palette().color (QPalette::Window));
			const int xstep = 3_screen_mm;
			const int ystep = 2.6_screen_mm;
			for (int x = 0; x < _double_buffer.size().width(); x += xstep)
			{
				for (int y = 0; y < _double_buffer.size().height(); y += ystep)
				{
					QPoint p (((y % (2 * ystep) == ystep) ? x : x + xstep / 2) + 3, y + 3);
					b.setPen (QPen (dark, 0.3_screen_mm));
					b.drawPoint (p);
					b.setPen (QPen (light, 0.3_screen_mm));
					b.drawPoint (p + QPoint (0.15_screen_mm, 0.15_screen_mm));
				}
			}
			break;
		}
	}

	QPainter (this).drawPixmap (paint_event->rect().topLeft(), _double_buffer, _double_buffer.rect());
}

} // namespace Haruhi

