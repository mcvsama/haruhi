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
#include <QtGui/QAbstractSlider>
#include <QtGui/QLayout>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "dial_control.h"


namespace Haruhi {

DialControl::DialControl (QWidget* parent, int value_min, int value_max, int value):
	QAbstractSlider (parent),
	_dial_pixmap ("share/images/dial.png"),
	_disabled_dial_pixmap ("share/images/dial:disabled.png"),
	_mouse_press_value (0),
	_to_update (false),
	_last_enabled_state (isEnabled()),
	_mouse_pressed (false),
	_ring_visible (false),
	_zero_value (0)
{
	setWindowFlags (Qt::WRepaintNoErase);
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	setFixedSize (37, 37);

	setMinimum (value_min);
	setMaximum (value_max);
	setValue (value);
	setPageStep (1);
	setSingleStep (1);
}


void
DialControl::paintEvent (QPaintEvent* paint_event)
{
	if (_to_update || _last_enabled_state != isEnabled())
	{
		_double_buffer.resize (width(), height());
		QColor indicator_color = isEnabled() ? QColor (0x5c, 0x64, 0x72) : QColor (0xd4, 0xd6, 0xda);
		QColor path1_color = isEnabled() ? QColor (0x2a, 0x41, 0x5b) : QColor (0xd4, 0xd6, 0xda);
		QColor path2_color = isEnabled() ? QColor (0xd1, 0xd3, 0xd6) : QColor (0xea, 0xec, 0xf0);

		QPainter b (&_double_buffer);
		b.setRenderHint (QPainter::Antialiasing, true);
		b.fillRect (rect(), backgroundColor());

		float pos_x = (width() - _dial_pixmap.width()) / 2.0f;
		float pos_y = (height() - _dial_pixmap.height()) / 2.0f;
		b.drawPixmap (pos_x, pos_y, isEnabled() ? _dial_pixmap : _disabled_dial_pixmap);

		float curr_angle = renormalize (value(), minValue(), maxValue(), -152, +152);
		if (_ring_visible)
		{
			float zero_angle = renormalize (_zero_value, minValue(), maxValue(), -152, +152);
			float span_angle = zero_angle - curr_angle;
			QRect r = rect().adjusted (1, 1, -1, -1);
			b.setPen (QPen (path2_color, 2));
			b.drawArc (r, (152 + 90) * 16, -2 * 152 * 16);
			b.setPen (QPen (path1_color, 2));
			b.drawArc (r, (-zero_angle + 90) * 16, span_angle * 16);
		}

		b.setPen (QPen (indicator_color, 2));
		b.translate (width() / 2, height() / 2);
		b.rotate (renormalize (value(), minValue(), maxValue(), -152, +152));
		b.drawLine (0, -4, 0, -11);
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
	setValue (_mouse_press_value + renormalize (_mouse_press_position.y() - event->pos().y(), 0, 200, 0, maxValue() - minValue()));
}


void
DialControl::wheelEvent (QWheelEvent* event)
{
	event->accept();
	float sgn = event->delta() > 0 ? 1.0f : -1.0f;
	float delta = 0.05f * sgn * (maxValue() - minValue());
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

} // namespace Haruhi

