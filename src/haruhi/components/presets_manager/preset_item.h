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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__PRESET_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__PRESET_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidgetItem>

// Local:
#include "preset.h"
#include "presets_manager.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

class CategoryItem;

class PresetItem: public QTreeWidgetItem
{
  public:
	PresetItem (CategoryItem* parent, Preset* preset);

	Preset*
	preset() const;

	CategoryItem*
	category_item() const;

	void
	reload();

	void
	read();

  private:
	void
	setup();

  private:
	Preset*	_preset;
};


inline Preset*
PresetItem::preset() const
{
	return _preset;
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

