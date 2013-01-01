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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__PACKAGE_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__PACKAGE_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "package.h"
#include "presets_manager.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

class Category;
class CategoryItem;

class PackageItem: public QTreeWidgetItem
{
  public:
	PackageItem (PresetsTree* parent, Package* package);

	Package*
	package() const;

	PresetsManager*
	presets_manager() const;

	void
	reload();

	void
	read();

	CategoryItem*
	create_category_item (Category* category);

	void
	remove_category_item (CategoryItem* category_item);

  private:
	void
	setup();

  private:
	PresetsManager*	_presets_manager;
	Package*		_package;
};


inline Package*
PackageItem::package() const
{
	return _package;
}


inline PresetsManager*
PackageItem::presets_manager() const
{
	return _presets_manager;
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

