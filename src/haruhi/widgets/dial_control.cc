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
#include <QAbstractSlider>
#include <QLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmapCache>
#include <QSvgRenderer>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "dial_control.h"


namespace Haruhi {

using namespace ScreenLiterals;


DialControl::DialControl (QWidget* parent, Range<int> value_range, int value):
	QAbstractSlider (parent),
	_mouse_press_value (0),
	_to_update (false),
	_last_enabled_state (isEnabled()),
	_mouse_pressed (false),
	_ring_visible (false),
	_center_value (0)
{
	setAttribute (Qt::WA_NoBackground);
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	setFixedSize (round_to_even (7.5_screen_mm), round_to_even (7.5_screen_mm));

	setMinimum (value_range.min());
	setMaximum (value_range.max());
	setValue (value);
	setPageStep (1);
	setSingleStep (1);

	// Render SVG into dial pixmaps:
	QSvgRenderer _dial_renderer (QString ("share/images/dial.svg"));

	// Enabled widget:
	QString enabled_key = "share/images/dial.svg:enabled";
	if (!QPixmapCache::find (enabled_key, _enabled_dial_pixmap))
	{
		_enabled_dial_pixmap = QPixmap (QSize (round_to_even (6.5_screen_mm), round_to_even (6.5_screen_mm)));
		QColor background = palette().color (QPalette::Window);
		QPainter painter (&_enabled_dial_pixmap);
		painter.fillRect (_enabled_dial_pixmap.rect(), background);
		_dial_renderer.render (&painter);
		QPixmapCache::insert (enabled_key, _enabled_dial_pixmap);
	}

	// Disabled widget:
	QString disabled_key = "share/images/dial.svg:disabled";
	if (!QPixmapCache::find (disabled_key, _disabled_dial_pixmap))
	{
		_disabled_dial_pixmap = QPixmap (QSize (round_to_even (6.5_screen_mm), round_to_even (6.5_screen_mm)));
		QColor background = palette().color (QPalette::Window);
		QPainter painter (&_disabled_dial_pixmap);
		painter.fillRect (_disabled_dial_pixmap.rect(), background);
		_dial_renderer.render (&painter);
		background.setAlphaF (0.75);
		painter.fillRect (_disabled_dial_pixmap.rect(), background);
		QPixmapCache::insert (disabled_key, _disabled_dial_pixmap);
	}
}


void
DialControl::paintEvent (QPaintEvent* paint_event)
{
	if (_to_update || _last_enabled_state != isEnabled())
	{
		_double_buffer = QPixmap (width(), height());
		QColor indicator_color = isEnabled() ? QColor (0x5c, 0x64, 0x72) : QColor (0xd4, 0xd6, 0xda);
		QColor path1_color = isEnabled() ? QColor (0x2a, 0x41, 0x5b) : QColor (0xd4, 0xd6, 0xda);
		QColor path2_color = isEnabled() ? QColor (0xd1, 0xd3, 0xd6) : QColor (0xea, 0xec, 0xf0);

		QPainter b (&_double_buffer);
		b.setRenderHint (QPainter::Antialiasing, true);
		b.fillRect (rect(), palette().color (QPalette::Window));

		float pos_x = (width() - _enabled_dial_pixmap.width()) / 2.0f;
		float pos_y = (height() - _enabled_dial_pixmap.height()) / 2.0f;
		b.drawPixmap (pos_x, pos_y, isEnabled() ? _enabled_dial_pixmap : _disabled_dial_pixmap);

		float curr_angle = renormalize (value(), minimum(), maximum(), -152, +152);
		if (_ring_visible)
		{
			float center_angle = renormalize (_center_value, minimum(), maximum(), -152, +152);
			float span_angle = center_angle - curr_angle;
			auto adj = 0.3_screen_mm;
			QRect r = rect().adjusted (adj, adj, -adj, -adj);
			b.setPen (QPen (path2_color, 0.4_screen_mm));
			b.drawArc (r, (152 + 90) * 16, -2 * 152 * 16);
			b.setPen (QPen (path1_color, 0.4_screen_mm));
			b.drawArc (r, (-center_angle + 90) * 16, span_angle * 16);
		}

		b.setPen (QPen (indicator_color, 0.3_screen_mm));
		b.translate (width() / 2, height() / 2);
		b.rotate (renormalize (value(), minimum(), maximum(), -152, +152));
		b.drawLine (0, -0.9_screen_mm, 0, -2.4_screen_mm);
	}
	QPainter (this).drawPixmap (paint_event->rect().topLeft(), _double_buffer, paint_event->rect());
	_last_enabled_state = isEnabled();
	_to_update = false;
}


void
DialControl::resizeEvent (QResizeEvent* resize_event)
{
	_to_update = true;
	QWidget::resizeEvent (resize_event);
}


void
DialControl::mousePressEvent (QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		event->accept();
		_mouse_press_position = event->pos();
		_mouse_press_value = value();
		_mouse_pressed = true;
	}
	else
		event->ignore();
}


void
DialControl::mouseReleaseEvent (QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		event->accept();
		_mouse_pressed = false;
	}
	else
		event->ignore();
}


void
DialControl::mouseMoveEvent (QMouseEvent* event)
{
	event->accept();
	setValue (_mouse_press_value + renormalize (_mouse_press_position.y() - event->pos().y(), 0, 200, 0, maximum() - minimum()));
}


void
DialControl::wheelEvent (QWheelEvent* event)
{
	event->accept();
	float sgn = event->delta() > 0 ? 1.0f : -1.0f;
	float delta = 0.05f * sgn * (maximum() - minimum());
	if (std::abs (delta) < 1.0f)
		delta = sgn;
	setValue (value() + static_cast<int> (delta));
}


void
DialControl::mouseDoubleClickEvent (QMouseEvent* event)
{
	event->ignore();
}


void
DialControl::sliderChange (SliderChange change)
{
	_to_update = true;
	update();
	if (change == SliderValueChange)
		emit valueChanged (value());
}


int
DialControl::round_to_even (float value)
{
	return static_cast<int> (value) / 2 * 2;
}

} // namespace Haruhi

