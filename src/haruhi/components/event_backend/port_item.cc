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

// Local:
#include "port_item.h"
#include "backend.h"


namespace Haruhi {

namespace EventBackendImpl {

PortItem::PortItem (Backend* backend):
	_backend (backend),
	_ready (false)
{
}

} // namespace EventBackendImpl

} // namespace Haruhi

