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
#include <QtGui/QApplication>

// Local:
#include "widgets.h"


namespace MikuruPrivate {

HorizontalLine::HorizontalLine (QWidget* parent):
	QFrame (parent, "<hr>", 0)
{
	setFrameStyle (QFrame::Sunken + QFrame::HLine);
	setFixedHeight (2 * Config::spacing + 2);
}


StyledLabel::StyledLabel (QString const& text, QWidget* parent):
	QLabel (text, parent)
{
	QFont font (QApplication::font());
	font.setWeight (QFont::Black);
	setFont (font);
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
	setAutoFillBackground (true);
	setForegroundRole (QPalette::Text);
	setBackgroundRole (QPalette::Dark);
	setFixedHeight (font.pointSize() + 10);
	setContentsMargins (2 * Config::margin, Config::margin + 1, 2 * Config::margin, Config::margin);
}


StyledCheckBoxLabel::StyledCheckBoxLabel (QString const& label, QWidget* parent):
	QWidget (parent)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setAutoFillBackground (true);
	setForegroundRole (QPalette::Text);
	setBackgroundRole (QPalette::Dark);
	setContentsMargins (0, 1, 0, 0);

	_checkbox = new QCheckBox (label, this);
	QFont font (QApplication::font());
	font.setWeight (QFont::Black);
	_checkbox->setFont (font);

	_label = new QLabel ("", this);
	_label->setFont (font);
	_label->setFixedHeight (font.pointSize() + 10);

	QHBoxLayout* layout = new QHBoxLayout (this, 0, 0);
	layout->addItem (new QSpacerItem (5, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
	layout->addWidget (_checkbox);
	layout->addWidget (_label);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
}

} // namespace MikuruPrivate

