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
#include <QtGui/QWidget>
#include <QtGui/QLayout>
#include <QtGui/QLabel>

// Haruhi:
#include <haruhi/config.h>

// Local:
#include "devices_manager.h"


namespace Haruhi {

DevicesManager::DevicesManager (QWidget* parent):
	QWidget (parent)
{
	QVBoxLayout* layout = new QVBoxLayout (this, Config::dialog_margin, Config::spacing);

	QLabel* info = new QLabel ("Device templates.", this);
	info->setMargin (Config::margin);
	layout->addWidget (info);

	QHBoxLayout* panels_layout = new QHBoxLayout (layout, Config::spacing);
	QHBoxLayout* input_buttons_layout = new QHBoxLayout (layout, Config::spacing);

//	panels_layout->addWidget (_inputs_list);
//	panels_layout->addWidget (_stack);
//
//	input_buttons_layout->addWidget (_create_device_button);
//	input_buttons_layout->addWidget (_create_controller_button);
//	input_buttons_layout->addWidget (_destroy_input_button);
//	input_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
//
//	selection_changed();
//	register_unit();
//
//	update_widgets();
}

} // namespace Haruhi

