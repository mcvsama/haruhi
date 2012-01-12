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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "category.h"
#include "preset_item.h"
#include "package_item.h"
#include "category_item.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

CategoryItem::CategoryItem (QTreeWidgetItem* parent, Category* category):
	QTreeWidgetItem (parent, QStringList(), Qt::ItemIsDragEnabled),
	_category (category)
{
	setup();
	read();
}


PackageItem*
CategoryItem::package_item() const
{
	QTreeWidgetItem* p = parent();
	if (p)
	{
		PackageItem* c = dynamic_cast<PackageItem*> (p);
		if (c)
			return c;
	}
	return 0;
}


void
CategoryItem::reload()
{
	setText (0, _category->name());
}


void
CategoryItem::read()
{
	reload();

	typedef std::set<Preset*> PresetsSet;

	// Read packages:
	PresetsSet m_presets; // Model presets
	PresetsSet t_presets; // TreeWidget items
	std::map<Preset*, PresetItem*> pi_by_p;

	for (Preset& p: _category->presets())
		m_presets.insert (&p);

	for (int i = 0; i < childCount(); ++i)
	{
		PresetItem* pi = dynamic_cast<PresetItem*> (child (i));
		if (!pi)
			continue;
		pi_by_p[pi->preset()] = pi;
		t_presets.insert (pi->preset());
	}

	PresetsSet added;
	PresetsSet removed;
	PresetsSet rest;
	std::set_difference (m_presets.begin(), m_presets.end(), t_presets.begin(), t_presets.end(), std::inserter (added, added.end()));
	std::set_difference (t_presets.begin(), t_presets.end(), m_presets.begin(), m_presets.end(), std::inserter (removed, removed.end()));
	std::set_intersection (m_presets.begin(), m_presets.end(), t_presets.begin(), t_presets.end(), std::inserter (rest, rest.end()));

	// Most safe is to remove items with removed packages first:
	for (Preset* p: removed)
		remove_preset_item (pi_by_p[p]);
	for (Preset* p: added)
		create_preset_item (p);
	for (Preset* p: rest)
		pi_by_p[p]->read();
}


PresetItem*
CategoryItem::create_preset_item (Preset* preset)
{
	return new PresetItem (this, preset);
}


void
CategoryItem::remove_preset_item (PresetItem* preset_item)
{
	removeChild (preset_item);
	delete preset_item;
}


void
CategoryItem::setup()
{
	setIcon (0, Resources::Icons16::presets_category());
	QSize s = sizeHint (0);
	if (s.height() < 18)
	{
		s.setHeight (18);
		setSizeHint (0, s);
	}
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

