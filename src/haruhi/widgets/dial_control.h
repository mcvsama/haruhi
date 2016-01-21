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

#ifndef HARUHI__WIDGETS__DIAL_CONTROL_H__INCLUDED
#define HARUHI__WIDGETS__DIAL_CONTROL_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QAbstractSlider>
#include <QtGui/QPixmap>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/range.h>


namespace Haruhi {

class DialControl: public QAbstractSlider
{
	Q_OBJECT

  public:
	DialControl (QWidget* parent, Range<int> value_range, int value);

	/**
	 * Enables additional value indicator - a ring drawn around
	 * the dial.
	 */
	void
	set_ring_visible (bool visible);

	/**
	 * Sets center (neutral) value for ring drawn around the dial.
	 */
	void
	set_center_value (int value);

	/**
	 * Returns true if mouse button is pressed over a dial,
	 * and user is to change dial's value.
	 */
	bool
	mouse_pressed() { return _mouse_pressed; }

  signals:
	void
	valueChanged (int);

  protected:
	void
	paintEvent (QPaintEvent*) override;

	void
	resizeEvent (QResizeEvent*) override;

	void
	mousePressEvent (QMouseEvent*) override;

	void
	mouseReleaseEvent (QMouseEvent*) override;

	void
	mouseMoveEvent (QMouseEvent*) override;

	void
	wheelEvent (QWheelEvent*) override;

	void
	mouseDoubleClickEvent (QMouseEvent*) override;

	void
	sliderChange (SliderChange change) override;

  private:
	static int
	round_to_even (float value);

  private:
	QPixmap		_dial_pixmap;
	QPixmap		_disabled_dial_pixmap;
	QPixmap		_double_buffer;
	QPoint		_mouse_press_position;
	int			_mouse_press_value;
	bool		_to_update;
	bool		_last_enabled_state;
	bool		_mouse_pressed;
	bool		_ring_visible;
	int			_center_value;
};


inline void
DialControl::set_ring_visible (bool visible)
{
	_ring_visible = visible;
}


inline void
DialControl::set_center_value (int value)
{
	_center_value = value;
}

} // namespace Haruhi

#endif

