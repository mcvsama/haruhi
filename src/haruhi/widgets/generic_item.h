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

#ifndef HARUHI__WIDGETS__TREE_WIDGET_ITEM_H__INCLUDED
#define HARUHI__WIDGETS__TREE_WIDGET_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QListWidgetItem>


namespace Haruhi {

extern void
make_standard_height (QTreeWidgetItem* item);

extern void
make_standard_height (QListWidgetItem* item);

} // namespace Haruhi

#endif

