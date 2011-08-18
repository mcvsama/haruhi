/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__PLUGINS__MIKURU__WIDGETS_H__INCLUDED
#define HARUHI__PLUGINS__MIKURU__WIDGETS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QLayout>

// Haruhi:
#include <haruhi/config/all.h>


namespace MikuruPrivate {

class HorizontalLine: public QFrame
{
  public:
	HorizontalLine (QWidget* parent);
};


class StyledLabel: public QLabel
{
  public:
	StyledLabel (QString const& text, QWidget* parent);
};


class StyledCheckBoxLabel: public QWidget
{
  public:
	StyledCheckBoxLabel (QString const& label, QWidget* parent);

	QCheckBox*
	checkbox() const { return _checkbox; }

	QLabel*
	label() const { return _label; }

  private:
	QCheckBox*	_checkbox;
	QLabel*		_label;
};

} // namespace MikuruPrivate

#endif

