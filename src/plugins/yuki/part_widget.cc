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
#include "part_widget.h"
#include "part_manager_widget.h"


namespace Yuki {

PartWidget::PartWidget (PartManagerWidget* part_manager_widget, Part* part):
	QWidget (part_manager_widget),
	_part_manager_widget (part_manager_widget),
	_part (part)
{
}

} // namespace Yuki
