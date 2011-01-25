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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__CATEGORY_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__CATEGORY_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidgetItem>

// Local:
#include "presets_manager.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

class PackageItem;

class CategoryItem: public QTreeWidgetItem
{
  public:
	CategoryItem (QString const& name, QTreeWidgetItem* parent);

	void
	setup();

	/**
	 * Returns PackageItem as a parent or 0.
	 */
	PackageItem*
	package_item() const;

	void
	reload() { }

	QString
	name() const { return text (0); }

	void
	set_name (QString const& name) { setText (0, name); }
};

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

