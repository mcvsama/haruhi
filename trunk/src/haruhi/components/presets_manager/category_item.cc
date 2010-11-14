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

// Standard:
#include <cstddef>

// Qt:
#include <Qt3Support/Q3ListView>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "package_item.h"
#include "category_item.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

CategoryItem::CategoryItem (QString const& name, Q3ListViewItem* parent):
	Q3ListViewItem (parent, name)
{
	setPixmap (0, Resources::Icons16::presets_category());
	setDragEnabled (true);
}


PackageItem*
CategoryItem::package_item() const
{
	Q3ListViewItem* p = parent();
	if (p)
	{
		PackageItem* c = dynamic_cast<PackageItem*> (p);
		if (c)
			return c;
	}
	return 0;
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

