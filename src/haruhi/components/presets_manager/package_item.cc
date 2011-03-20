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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "helpers.h"
#include "package.h"
#include "category.h"
#include "package_item.h"
#include "category_item.h"
#include "preset_item.h"
#include "presets_tree.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

PackageItem::PackageItem (PresetsTree* parent, Package* package):
	QTreeWidgetItem (parent),
	_presets_manager (parent->presets_manager()),
	_package (package)
{
	setup();
	read();
}


void
PackageItem::reload()
{
	setText (0, _package->name());
}


void
PackageItem::read()
{
	reload();

	typedef std::set<Category*> CategoriesSet;

	// Read packages:
	CategoriesSet m_categories; // Model categories
	CategoriesSet t_categories; // TreeWidget items
	std::map<Category*, CategoryItem*> ci_by_c;

	for (Package::Categories::iterator p = _package->categories().begin(); p != _package->categories().end(); ++p)
		m_categories.insert (&*p);

	for (int i = 0; i < childCount(); ++i)
	{
		CategoryItem* ci = dynamic_cast<CategoryItem*> (child (i));
		if (!ci)
			continue;
		ci_by_c[ci->category()] = ci;
		t_categories.insert (ci->category());
	}

	CategoriesSet added;
	CategoriesSet removed;
	CategoriesSet rest;
	std::set_difference (m_categories.begin(), m_categories.end(), t_categories.begin(), t_categories.end(), std::inserter (added, added.end()));
	std::set_difference (t_categories.begin(), t_categories.end(), m_categories.begin(), m_categories.end(), std::inserter (removed, removed.end()));
	std::set_intersection (m_categories.begin(), m_categories.end(), t_categories.begin(), t_categories.end(), std::inserter (rest, rest.end()));

	// Most safe is to remove items with removed packages first:
	for (CategoriesSet::iterator c = removed.begin(); c != removed.end(); ++c)
		remove_category_item (ci_by_c[*c]);
	for (CategoriesSet::iterator c = added.begin(); c != added.end(); ++c)
		create_category_item (*c);
	for (CategoriesSet::iterator c = rest.begin(); c != rest.end(); ++c)
		ci_by_c[*c]->read();
}


CategoryItem*
PackageItem::create_category_item (Category* category)
{
	return new CategoryItem (this, category);
}


void
PackageItem::remove_category_item (CategoryItem* category_item)
{
	removeChild (category_item);
	delete category_item;
}


void
PackageItem::setup()
{
	setIcon (0, Resources::Icons16::presets_package());
	QSize s = sizeHint (0);
	if (s.height() < 18)
	{
		s.setHeight (18);
		setSizeHint (0, s);
	}
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

