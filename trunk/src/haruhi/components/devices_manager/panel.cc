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
#include <QtGui/QMenu>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "panel.h"
#include "device_dialog.h"
#include "device_item.h"
#include "controller_dialog.h"
#include "controller_item.h"


namespace Haruhi {

namespace DevicesManager {

Panel::Panel (QWidget* parent):
	QWidget (parent)
{
	_tree = new PortsListView (this);

	QObject::connect (_tree, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_items (const QPoint&)));
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
	QObject::connect (_destroy_input_button, SIGNAL (clicked()), this, SLOT (destroy_selected_item()));

	// Right panel (stack):

	_stack = new QStackedWidget (this);
	_device_dialog = new DeviceDialog (this);
	_controller_dialog = new ControllerDialog (this);
	_stack->addWidget (_device_dialog);
	_stack->addWidget (_controller_dialog);
	_stack->setCurrentWidget (_device_dialog);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::Margin, Config::Spacing);
	QHBoxLayout* input_buttons_layout = new QHBoxLayout (layout, Config::Spacing);
	QHBoxLayout* panels_layout = new QHBoxLayout (layout, Config::Spacing);

	QLabel* info = new QLabel ("Device templates.", this);
	info->setMargin (Config::Margin);
	layout->addWidget (info);

	panels_layout->addWidget (_tree);
	panels_layout->addWidget (_stack);

	input_buttons_layout->addWidget (_create_device_button);
	input_buttons_layout->addWidget (_create_controller_button);
	input_buttons_layout->addWidget (_destroy_input_button);
	input_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	selection_changed();
	update_widgets();
}


void
Panel::update_widgets()
{
	QTreeWidgetItem* sel = _tree->selected_item();
	_create_controller_button->setEnabled (sel != 0);
	_destroy_input_button->setEnabled (sel != 0);

	// "Destroy device" or "Destroy controller":
	if (sel)
	{
		if (dynamic_cast<DeviceItem*> (sel))
			_destroy_input_button->setText ("Destroy device");
		else if (dynamic_cast<ControllerItem*> (sel))
			_destroy_input_button->setText ("Destroy controller");
		else
			_destroy_input_button->setText ("Destroy");
	}
}


void
Panel::selection_changed()
{
	update_widgets();
	configure_selected_item();
}


void
Panel::create_device()
{
	QString name = "<unnamed device>";
	QTreeWidgetItem* item = _tree->create_device_item (name);
	_tree->setCurrentItem (item);
}


void
Panel::create_controller()
{
	QString name = "<unnamed controller>";
	QTreeWidgetItem* sel = _tree->selected_item();
	if (sel != 0)
	{
		DeviceItem* parent = dynamic_cast<DeviceItem*> (sel);
		if (parent == 0)
			parent = dynamic_cast<DeviceItem*> (sel->parent());
		if (parent != 0)
		{
			QTreeWidgetItem* item = parent->create_controller_item (name);
			_tree->setCurrentItem (item);
			parent->setExpanded (true);
		}
	}
}


void
Panel::configure_item (DeviceItem* item)
{
	_device_dialog->from (item);
	_stack->setCurrentWidget (_device_dialog);
}


void
Panel::configure_item (ControllerItem* item)
{
	_controller_dialog->from (item);
	_stack->setCurrentWidget (_controller_dialog);
}


void
Panel::configure_selected_item()
{
	if (_tree->selected_item())
	{
		DeviceItem* device_item = dynamic_cast<DeviceItem*> (_tree->selected_item());
		if (device_item)
			configure_item (device_item);
		else
		{
			ControllerItem* controller_item = dynamic_cast<ControllerItem*> (_tree->selected_item());
			if (controller_item)
				configure_item (controller_item);
		}
	}
	else
	{
		_device_dialog->clear();
		_controller_dialog->clear();
	}
}


void
Panel::learn_from_midi()
{
	if (_tree->selected_item())
	{
		ControllerItem* item = dynamic_cast<ControllerItem*> (_tree->selected_item());
		if (item)
			item->learn();
	}
}


void
Panel::destroy_selected_item()
{
	if (_tree->selected_item())
	{
		QTreeWidgetItem* item = _tree->selected_item();
		if (item->parent())
			item->parent()->takeChild (item->parent()->indexOfChild (item));
		delete item;
	}
}


void
Panel::context_menu_for_items (QPoint const& pos)
{
	QMenu* menu = new QMenu (this);
	QTreeWidgetItem* item = _tree->itemAt (pos);
	QAction* a;

	if (item != 0)
	{
		if (dynamic_cast<ControllerItem*> (item) != 0)
		{
			menu->addAction (Config::Icons16::colorpicker(), "&Learn", this, SLOT (learn_from_midi()));
			menu->addSeparator();
			menu->addAction (Config::Icons16::add(), "Add &controller", this, SLOT (create_controller()));
			menu->addAction (Config::Icons16::remove(), "&Destroy", this, SLOT (destroy_selected_item()));
		}
		else if (dynamic_cast<DeviceItem*> (item) != 0)
		{
			menu->addAction (Config::Icons16::add(), "Add &controller", this, SLOT (create_controller()));
			menu->addSeparator();
			menu->addAction (Config::Icons16::add(), "&Add device", this, SLOT (create_device()));
			menu->addAction (Config::Icons16::remove(), "&Destroy", this, SLOT (destroy_selected_item()));
		}
	}
	else
	{
		menu->addAction (Config::Icons16::add(), "&Add device", this, SLOT (create_device()));
		a = menu->addAction (Config::Icons16::remove(), "&Destroy", this, SLOT (destroy_selected_item()));
		a->setEnabled (false);
	}

	menu->exec (QCursor::pos());
	delete menu;
}

} // namespace DevicesManager

} // namespace Haruhi

