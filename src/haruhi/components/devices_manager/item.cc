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
#include <QtGui/QMessageBox>
#include <QtGui/QHeaderView>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/widgets/generic_item.h>

// Local:
#include "item.h"


namespace Haruhi {

namespace DevicesManager {

void
Item::update_minimum_size()
{
	make_standard_height (this);
}

} // namespace DevicesManager

} // namespace Haruhi

