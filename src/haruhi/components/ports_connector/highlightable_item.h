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

#ifndef HARUHI__COMPONENTS__PORTS_CONNECTOR__HIGHLIGHTABLE_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__PORTS_CONNECTOR__HIGHLIGHTABLE_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QTreeWidgetItem>
#include <QBrush>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

namespace PortsConnectorPrivate {

class HighlightableItem: virtual public QTreeWidgetItem
{
  public:
	HighlightableItem();

	void
	set_highlighted (bool set);

  public slots:
	void
	update_highlight();

  private:
	bool	_highlighted;
	int		_highlighted_subitems;
	QBrush	_normal_foreground;
	QBrush	_normal_background;
	QBrush	_highlighted_foreground;
	QBrush	_highlighted_background;
};

} // namespace PortsConnectorPrivate

} // namespace Haruhi

#endif

