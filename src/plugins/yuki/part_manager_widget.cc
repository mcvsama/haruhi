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
#include "part_manager_widget.h"
#include "part_manager.h"


namespace Yuki {

PartManagerWidget::PartManagerWidget (QWidget* parent, PartManager* part_manager):
	QWidget (parent),
	_part_manager (part_manager)
{
}


Plugin*
PartManagerWidget::plugin() const
{
	return _part_manager->plugin();
}

} // namespace Yuki

