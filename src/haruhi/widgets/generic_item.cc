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

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>

// Local:
#include "generic_item.h"


namespace Haruhi {

using namespace Haruhi::ScreenLiterals;


void
make_standard_height (QTreeWidgetItem* item)
{
	QSize s = item->sizeHint (0);

	if (s.height() < 4.5_screen_mm)
	{
		s.setHeight (4.5_screen_mm);
		item->setSizeHint (0, s);
	}
}


void
make_standard_height (QListWidgetItem* item)
{
	QSize s = item->sizeHint();

	if (s.height() < 4.5_screen_mm)
	{
		s.setHeight (4.5_screen_mm);
		item->setSizeHint (s);
	}
}

} // namespace Haruhi

