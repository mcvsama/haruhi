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

#ifndef HARUHI__WIDGETS__CLICKABLE_LABEL_H__INCLUDED
#define HARUHI__WIDGETS__CLICKABLE_LABEL_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QLabel>


namespace Haruhi {

/**
 * Implements label that sends signal clicked() when clicked.
 */
class ClickableLabel: public QLabel
{
	Q_OBJECT

  public:
	ClickableLabel (QString const& text, QWidget* parent, const char* name = 0, Qt::WFlags f = 0);

  protected:
	void
	mousePressEvent (QMouseEvent*);

	void
	mouseReleaseEvent (QMouseEvent*);

  signals:
	void
	clicked();

  private:
	Qt::ButtonState _button_state;
};

} // namespace Haruhi

#endif

