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

// Local:
#include "controller_with_port_dialog.h"
#include "controller_with_port_item.h"


namespace Haruhi {

namespace EventBackendImpl {

ControllerWithPortDialog::ControllerWithPortDialog (QWidget* parent):
	ControllerDialog (parent)
{
}


void
ControllerWithPortDialog::apply (ControllerItem* item) const
{
	ControllerDialog::apply (item);
	auto controller_item = dynamic_cast<ControllerWithPortItem*> (item);
	if (controller_item)
		controller_item->update_name();
}

} // namespace EventBackendImpl

} // namespace Haruhi

