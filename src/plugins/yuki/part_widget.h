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

#ifndef HARUHI__PLUGINS__YUKI__PART_WIDGET_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>

// Qt:
#include <QtGui/QWidget>

// Haruhi:
#include <haruhi/config/all.h>


namespace Yuki {

class Part;
class PartManagerWidget;

class PartWidget: public QWidget
{
	Q_OBJECT

  public:
	PartWidget (PartManagerWidget*, Part*);

	Part*
	part() const;

  private:
	PartManagerWidget*	_part_manager_widget;
	Part*				_part;
};


inline Part*
PartWidget::part() const
{
	return _part;
}

} // namespace Yuki

#endif

