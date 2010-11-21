/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
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
	_last_enabled_state (isEnabled())
{
	setWindowFlags (Qt::WRepaintNoErase);
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	setFixedSize (33, 33);

	setMinimum (value_min);
	setMaximum (value_max);
	setValue (value);
	setPageStep (1);
	setSingleStep (1);
}


DialControl::~DialControl()
{
}


void
DialControl::paintEvent (QPaintEvent* paint_event)
{
	if (_to_update || _last_enabled_state != isEnabled())
	{
		_double_buffer.resize (width(), height());

		QPainter b (&_double_buffer);
		b.setRenderHint (QPainter::Antialiasing, true);
		b.fillRect (rect(), backgroundColor());
		b.drawPixmap (0, 0, isEnabled() ? _dial_pixmap : _disabled_dial_pixmap);
		b.setPen (QPen (isEnabled() ? QColor (0x5c, 0x64, 0x72) : QColor (0xd4, 0xd6, 0xda), 2));
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

