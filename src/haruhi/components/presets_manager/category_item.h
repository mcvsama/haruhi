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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__CATEGORY_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__CATEGORY_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "category.h"
#include "presets_manager.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

class Preset;
class PresetItem;
class PackageItem;

class CategoryItem: public QTreeWidgetItem
{
  public:
	CategoryItem (QTreeWidgetItem* parent, Category* category);

	Category*
	category() const;

	PackageItem*
	package_item() const;

	void
	reload();

	void
	read();

	PresetItem*
	create_preset_item (Preset* preset);

	void
	remove_preset_item (PresetItem* preset_item);

  private:
	void
	setup();

  private:
	Category*	_category;
};


inline Category*
CategoryItem::category() const
{
	return _category;
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

