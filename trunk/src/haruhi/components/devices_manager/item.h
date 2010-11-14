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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__ITEM_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/utility/atomic.h>

// Local:
#include "ports_list_view.h"


namespace Haruhi {

namespace DevicesManager {

class Item: public QTreeWidgetItem
{
  public:
	template<class Parent>
		Item (Parent* parent, QString const& name):
			QTreeWidgetItem (parent, QStringList (name))
		{ }

	void
	update_minimum_size();
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

