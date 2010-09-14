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

#ifndef HARUHI__WIDGETS__DIAL_CONTROL_H__INCLUDED
#define HARUHI__WIDGETS__DIAL_CONTROL_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <Qt3Support/Q3RangeControl>


class DialControl:
	public QWidget,
	public Q3RangeControl
{
	Q_OBJECT

  public:
	DialControl (QWidget* parent, int value_min, int value_max, int value);

	~DialControl();

  signals:
	void
	valueChanged (int);

  protected:
	void
	paintEvent (QPaintEvent*);

	void
	resizeEvent (QResizeEvent*);

	void
	mousePressEvent (QMouseEvent*);

	void
	mouseMoveEvent (QMouseEvent*);

	void
	wheelEvent (QWheelEvent*);

	void
	mouseDoubleClickEvent (QMouseEvent*);

	void
	valueChange();

	void
	rangeChange();

  private:
	QPixmap		_dial_pixmap;
	QPixmap		_disabled_dial_pixmap;
	QPixmap		_double_buffer;
	QPoint		_mouse_press_position;
	int			_mouse_press_value;
	bool		_to_update;
	bool		_last_enabled_state;
};

#endif

