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
#include <QtGui/QToolTip>

// Haruhi:
#include <haruhi/config.h>

// Local:
#include "devices_manager.h"


namespace Haruhi {

namespace Private = DevicesManagerPrivate;

DevicesManager::DevicesManager (QWidget* parent):
	QWidget (parent)
{
	_tree = new Private::PortsListView (this);

	QObject::connect (_tree, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_inputs (const QPoint&)));
	QObject::connect (_tree, SIGNAL (itemSelectionChanged()), this, SLOT (selection_changed()));

	_create_device_button = new QPushButton (Config::Icons16::add(), "Add device", this);
	_create_device_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_create_device_button, "Add new device and external input port");

	_create_controller_button = new QPushButton (Config::Icons16::add(), "Add controller", this);
	_create_controller_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_create_controller_button, "Add new controller and internal output port");

	_destroy_input_button = new QPushButton (Config::Icons16::remove(), "Destroy device", this);
	_destroy_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_destroy_input_button, "Destroy selected device or controller");

	QObject::connect (_create_device_button, SIGNAL (clicked()), this, SLOT (create_device()));
	QObject::connect (_create_controller_button, SIGNAL (clicked()), this, SLOT (create_controller()));
	QObject::connect (_destroy_input_button, SIGNAL (clicked()), this, SLOT (destroy_selected_input()));

	// Right panel (stack):

//	_stack = new QStackedWidget (this);
//	_device_dialog = new Private::DeviceDialog (this);
//	_controller_dialog = new Private::ControllerDialog (this);
//	_stack->addWidget (_device_dialog);
//	_stack->addWidget (_controller_dialog);
//	_stack->setCurrentWidget (_device_dialog);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::margin, Config::spacing);
	QHBoxLayout* input_buttons_layout = new QHBoxLayout (layout, Config::spacing);
	QHBoxLayout* panels_layout = new QHBoxLayout (layout, Config::spacing);

	QLabel* info = new QLabel ("Device templates.", this);
	info->setMargin (Config::margin);
	layout->addWidget (info);

	panels_layout->addWidget (_tree);
	panels_layout->addWidget (_stack);

	input_buttons_layout->addWidget (_create_device_button);
	input_buttons_layout->addWidget (_create_controller_button);
	input_buttons_layout->addWidget (_destroy_input_button);
	input_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	//selection_changed();
	//update_widgets();
}

} // namespace Haruhi

