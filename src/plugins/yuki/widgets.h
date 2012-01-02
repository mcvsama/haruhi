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

#ifndef HARUHI__PLUGINS__YUKI__WIDGETS_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__WIDGETS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>

// Haruhi:
#include <haruhi/config/all.h>


namespace Yuki {

class StyledBackground: public QWidget
{
  public:
	StyledBackground (QWidget* child, QWidget* parent, int margin = 0);
};

} // namespace Yuki

#endif

