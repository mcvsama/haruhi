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
#include <QtGui/QMouseEvent>

// Local:
#include "clickable_label.h"


namespace Haruhi {

ClickableLabel::ClickableLabel (QString const& text, QWidget* parent):
	QLabel (text, parent),
	_button_state (Qt::NoButton)
{
}


void
ClickableLabel::mousePressEvent (QMouseEvent* event)
{
	event->accept();
	_button_state = Qt::LeftButton;
}


void
ClickableLabel::mouseReleaseEvent (QMouseEvent* event)
{
	event->accept();
	if (event->x() >= 0 && event->x() < this->width() && event->y() >= 0 && event->y() < this->height())
	{
		if (_button_state == Qt::LeftButton)
			emit clicked();
	}
	_button_state = Qt::NoButton;
}

} // namespace Haruhi
