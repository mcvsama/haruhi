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
#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QMessageBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/lib/midi.h>

// Local:
#include "panel.h"
#include "device_dialog.h"
#include "device_item.h"
#include "controller_dialog.h"
#include "controller_item.h"
#include "model.h"
#include "tree.h"


namespace Haruhi {

namespace DevicesManager {

Panel::Panel (QWidget* parent, Settings* settings):
	QWidget (parent),
	_settings (settings),
	_event_backend (0)
{
	_tree = new Tree (this, &settings->model());
	QObject::connect (_tree, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_items (const QPoint&)));
	QObject::connect (_tree, SIGNAL (itemSelectionChanged()), this, SLOT (selection_changed()));

	_create_device_button = new QPushButton (Resources::Icons16::add(), "Add device", this);
	_create_device_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_create_device_button->setToolTip ("Add new device and external input port");
	_create_device_button->setFlat (true);

	_create_controller_button = new QPushButton (Resources::Icons16::add(), "Add controller", this);
	_create_controller_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_create_controller_button->setToolTip ("Add new controller and internal output port");
	_create_controller_button->setFlat (true);

	_destroy_input_button = new QPushButton (Resources::Icons16::remove(), "Destroy device", this);
	_destroy_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_destroy_input_button->setToolTip ("Destroy selected device or controller");
	_destroy_input_button->setFlat (true);

	QObject::connect (_create_device_button, SIGNAL (clicked()), _tree, SLOT (create_device()));
	QObject::connect (_create_controller_button, SIGNAL (clicked()), _tree, SLOT (create_controller()));
	QObject::connect (_destroy_input_button, SIGNAL (clicked()), _tree, SLOT (destroy_selected_item()));

	_device_dialog = new DeviceDialog (this, DeviceDialog::DisplayAutoAdd);
	QObject::connect (_device_dialog, SIGNAL (item_configured (DeviceItem*)), this, SLOT (save_settings()));

	_controller_dialog = new ControllerDialog (this);
	QObject::connect (_controller_dialog, SIGNAL (item_configured (ControllerItem*)), this, SLOT (save_settings()));

	_stack = new QStackedWidget (this);
	_stack->addWidget (_device_dialog);
	_stack->addWidget (_controller_dialog);
	_stack->setCurrentWidget (_device_dialog);

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::spacing());
	buttons_layout->addWidget (_create_device_button);
	buttons_layout->addWidget (_create_controller_button);
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (_destroy_input_button);

	QGridLayout* layout = new QGridLayout (this);
	layout->setMargin (Config::margin());
	layout->setSpacing (Config::spacing());
	layout->addLayout (buttons_layout, 0, 0);
	layout->addWidget (_tree, 1, 0);
	layout->addWidget (_stack, 1, 1);

	selection_changed();
	update_widgets();

	_tree->read_model();
}


Panel::~Panel()
{
	Signal::Receiver::disconnect_all_signals();
}


void
Panel::on_event (MIDI::Event const& event)
{
	if (!_learning_items.empty())
		for (auto& li: _learning_items)
			if (li->learn_from_event (event))
				_learning_items.erase (li);
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
	QTreeWidgetItem* item = _tree->selected_item();
	if (item)
	{
		DeviceItem* device_item = dynamic_cast<DeviceItem*> (item);
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
	QTreeWidgetItem* item = _tree->selected_item();
	if (item)
	{
		if (_event_backend && _event_backend->transport()->learning_possible())
		{
			ControllerItem* controller_item = dynamic_cast<ControllerItem*> (item);
			if (controller_item)
			{
				controller_item->learn();
				_learning_items.insert (controller_item);
			}
		}
		else
		{
			QMessageBox::information (this,
									  "Connect input device", "First, connect a real input device to any of Haruhi external ports.\n\n"
									  "You can use QJackCtl to do this.");
		}
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
		ControllerItem* controller_item = dynamic_cast<ControllerItem*> (item);
		if (controller_item != 0)
		{
			menu->addAction (Resources::Icons16::colorpicker(), controller_item->learning() ? "Stop &learning" : "&Learn", this, SLOT (learn_from_midi()));
			menu->addSeparator();
			menu->addAction (Resources::Icons16::add(), "Add &controller", _tree, SLOT (create_controller()));
			menu->addAction (Resources::Icons16::remove(), "&Destroy", _tree, SLOT (destroy_selected_item()));
		}
		else if (dynamic_cast<DeviceItem*> (item) != 0)
		{
			menu->addAction (Resources::Icons16::add(), "Add &controller", _tree, SLOT (create_controller()));
			menu->addSeparator();
			menu->addAction (Resources::Icons16::add(), "&Add device", _tree, SLOT (create_device()));
			menu->addAction (Resources::Icons16::remove(), "&Destroy", _tree, SLOT (destroy_selected_item()));
		}
	}
	else
	{
		menu->addAction (Resources::Icons16::add(), "&Add device", _tree, SLOT (create_device()));
		a = menu->addAction (Resources::Icons16::remove(), "&Destroy", _tree, SLOT (destroy_selected_item()));
		a->setEnabled (false);
	}

	menu->exec (QCursor::pos());
	delete menu;
}


void
Panel::save_settings()
{
	_settings->save();
	_settings->model().changed();
}

} // namespace DevicesManager

} // namespace Haruhi

