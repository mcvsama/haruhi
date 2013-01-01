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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "helpers.h"
#include "preset.h"
#include "category_item.h"
#include "preset_item.h"
#include "presets_tree.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

PresetItem::PresetItem (CategoryItem* parent, Preset* preset):
	QTreeWidgetItem (parent, QStringList(), Qt::ItemIsDragEnabled),
	_preset (preset)
{
	setup();
	read();
}


CategoryItem*
PresetItem::category_item() const
{
	QTreeWidgetItem* p = parent();
	if (p)
	{
		CategoryItem* c = dynamic_cast<CategoryItem*> (p);
		if (c)
			return c;
	}
	return 0;
}


void
PresetItem::reload()
{
	setText (0, _preset->name());
}


void
PresetItem::read()
{
	reload();
}


void
PresetItem::setup()
{
	setIcon (0, Resources::Icons16::preset());
	QSize s = sizeHint (0);
	if (s.height() < 18)
	{
		s.setHeight (18);
		setSizeHint (0, s);
	}
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

