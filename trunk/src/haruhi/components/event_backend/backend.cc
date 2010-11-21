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
#include <QtGui/QPushButton>
#include <QtGui/QToolTip>
#include <QtGui/QMessageBox>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QMenu>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/components/devices_manager/device_dialog.h>
#include <haruhi/components/devices_manager/controller_dialog.h>
#include <haruhi/graph/event_buffer.h>

// Local:
#include "transports/alsa_transport.h"
#include "backend.h"
#include "device_with_port_dialog.h"
#include "controller_with_port_dialog.h"
#include "controller_with_port_item.h"


namespace Haruhi {

namespace EventBackendImpl {

Backend::Backend (QString const& client_name, int id, QWidget* parent):
	QWidget (parent),
	EventBackend ("• Event", id),
	_client_name (client_name),
	_insert_template_signal_mapper (0),
	_tree (0),
	_templates_menu (0)
{
	_transport = new AlsaTransport (this);

	//
	// Widgets
	//

	// Ports list:

	_tree = new PortsListView (this, this);

	QObject::connect (_tree, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_items (const QPoint&)));
	QObject::connect (_tree, SIGNAL (itemSelectionChanged()), this, SLOT (selection_changed()));

	_create_device_button = new QPushButton (Resources::Icons16::add(), "Add device", this);
	_create_device_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_create_device_button, "Add new device and external input port");

	_create_controller_button = new QPushButton (Resources::Icons16::add(), "Add controller", this);
	_create_controller_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_create_controller_button, "Add new controller and internal output port");

	_destroy_input_button = new QPushButton (Resources::Icons16::remove(), "Destroy device", this);
	_destroy_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_destroy_input_button, "Destroy selected device or controller");

	QObject::connect (_create_device_button, SIGNAL (clicked()), this, SLOT (create_device()));
	QObject::connect (_create_controller_button, SIGNAL (clicked()), this, SLOT (create_controller()));
	QObject::connect (_destroy_input_button, SIGNAL (clicked()), this, SLOT (destroy_selected_item()));

	// Right panel (stack):

	_stack = new QStackedWidget (this);
	_device_dialog = new DeviceWithPortDialog (this);
	_controller_dialog = new ControllerWithPortDialog (this);
	_stack->addWidget (_device_dialog);
	_stack->addWidget (_controller_dialog);
	_stack->setCurrentWidget (_device_dialog);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::Margin, Config::Spacing);
	QHBoxLayout* input_buttons_layout = new QHBoxLayout (layout, Config::Spacing);
	QHBoxLayout* panels_layout = new QHBoxLayout (layout, Config::Spacing);

	QLabel* info = new QLabel ("Devices used in current session. Each device corresponds to external MIDI port.", this);
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


Backend::~Backend()
{
	delete _templates_menu;

	_tree->disconnect();
	_create_device_button->disconnect();
	_create_controller_button->disconnect();
	_destroy_input_button->disconnect();
}


void
Backend::registered()
{
	connect();
}


void
Backend::unregistered()
{
	if (connected())
		disconnect();

	_tree->clear();
}


void
Backend::process()
{
	// Sync all inputs:
	sync_inputs();
	clear_outputs();

	// Sync transport ports:
	_transport->sync();

	for (InputsMap::iterator h = _inputs.begin(); h != _inputs.end(); ++h)
	{
		Transport::Port* transport_port = h->first;
		if (transport_port->buffer().empty())
			continue;
		DeviceWithPortItem* device_item = h->second;
		for (DeviceWithPortItem::Controllers::iterator iii = device_item->controllers()->begin(); iii != device_item->controllers()->end(); ++iii)
		{
			if ((*iii)->ready())
			{
				for (Transport::MidiBuffer::iterator m = transport_port->buffer().begin(); m != transport_port->buffer().end(); ++m)
				{
					if ((*iii)->handle_event (*m))
						handle_event_for_learnables (*m, (*iii)->port());
				}
			}
		}
	}
}


void
Backend::save_state (QDomElement& element) const
{
	QDomElement inputs = element.ownerDocument().createElement ("inputs");
	_tree->save_state (inputs);
	element.appendChild (inputs);
}


void
Backend::load_state (QDomElement const& element)
{
	bool e = enabled();
	if (e)
		disable();
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "inputs")
				_tree->load_state (e);
		}
	}
	if (e)
		enable();
}


void
Backend::connect()
{
	try {
		_transport->connect (_client_name.toStdString());
		enable();
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Event backend", QString ("Can't connect to event backend: ") + e.what());
	}
}


void
Backend::disconnect()
{
	disable();
	_transport->disconnect();
}


bool
Backend::connected() const
{
	return _transport->connected();
}


void
Backend::update_widgets()
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
Backend::selection_changed()
{
	update_widgets();
	configure_selected_item();
}


void
Backend::create_device()
{
	QString name = "<unnamed device>";
	QTreeWidgetItem* item = _tree->create_device_item (name);
	_tree->setCurrentItem (item);
}


void
Backend::create_controller()
{
	QString name = "<unnamed controller>";
	QTreeWidgetItem* sel = _tree->selected_item();
	if (sel != 0)
	{
		DeviceWithPortItem* parent = dynamic_cast<DeviceWithPortItem*> (sel);
		if (parent == 0)
			parent = dynamic_cast<DeviceWithPortItem*> (sel->parent());
		if (parent != 0)
		{
			QTreeWidgetItem* item = parent->create_controller_item (name);
			_tree->setCurrentItem (item);
			parent->setExpanded (true);
		}
	}
}


void
Backend::configure_item (DeviceItem* item)
{
	_device_dialog->from (item);
	_stack->setCurrentWidget (_device_dialog);
}


void
Backend::configure_item (ControllerItem* item)
{
	_controller_dialog->from (item);
	_stack->setCurrentWidget (_controller_dialog);
}


void
Backend::configure_selected_item()
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
Backend::learn_from_midi()
{
	if (_tree->selected_item())
	{
		ControllerWithPortItem* item = dynamic_cast<ControllerWithPortItem*> (_tree->selected_item());
		if (item)
			item->learn();
	}
}


void
Backend::destroy_selected_item()
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
Backend::save_selected_item()
{
	QTreeWidgetItem* item = _tree->selected_item();
	if (item)
	{
		DeviceWithPortItem* input_item = dynamic_cast<DeviceWithPortItem*> (item);
		if (input_item)
		{
			Settings::EventHardwareTemplate tpl (input_item->name());
			input_item->save_state (tpl.element);
			Settings::event_hardware_templates().push_back (tpl);
			Settings::save_event_hardware_templates();
			QMessageBox::information (this, "Created template", "Created new template \"" + input_item->name() + "\".");
		}
	}
}


void
Backend::context_menu_for_items (QPoint const& pos)
{
	QMenu* menu = new QMenu (this);
	QTreeWidgetItem* item = _tree->itemAt (pos);
	QAction* a;

	if (item != 0)
	{
		if (dynamic_cast<ControllerItem*> (item) != 0)
		{
			menu->addAction (Resources::Icons16::colorpicker(), "&Learn", this, SLOT (learn_from_midi()));
			menu->addSeparator();
			menu->addAction (Resources::Icons16::add(), "Add &controller", this, SLOT (create_controller()));
			menu->addAction (Resources::Icons16::remove(), "&Destroy", this, SLOT (destroy_selected_item()));
		}
		else if (dynamic_cast<DeviceItem*> (item) != 0)
		{
			menu->addAction (Resources::Icons16::add(), "Add &controller", this, SLOT (create_controller()));
			menu->addSeparator();
			menu->addAction (Resources::Icons16::save(), "&Save as template", this, SLOT (save_selected_item()));
			menu->addSeparator();
			menu->addAction (Resources::Icons16::add(), "&Add device", this, SLOT (create_device()));
			menu->addAction (Resources::Icons16::remove(), "&Destroy", this, SLOT (destroy_selected_item()));
		}
	}
	else
	{
		menu->addAction (Resources::Icons16::add(), "&Add device", this, SLOT (create_device()));
		a = menu->addAction (Resources::Icons16::remove(), "&Destroy", this, SLOT (destroy_selected_item()));
		a->setEnabled (false);
	}
	menu->addSeparator();
	QMenu* templates_menu = menu->addMenu (Resources::Icons16::insert(), "&Insert template");
	create_templates_menu (templates_menu);

	menu->exec (QCursor::pos());
	delete menu;
}


void
Backend::create_templates_menu (QMenu* menu)
{
	if (_insert_template_signal_mapper)
		delete _insert_template_signal_mapper;
	_insert_template_signal_mapper = new QSignalMapper (this);
	QObject::connect (_insert_template_signal_mapper, SIGNAL (mapped (int)), this, SLOT (insert_template (int)));

	int action_id = 0;
	_templates.clear();
	for (Settings::EventHardwareTemplates::iterator t = Settings::event_hardware_templates().begin(); t != Settings::event_hardware_templates().end(); ++t)
	{
		action_id += 1;
		QAction* a = menu->addAction (Resources::Icons16::template_(), t->name, _insert_template_signal_mapper, SLOT (map()));
		_insert_template_signal_mapper->setMapping (a, action_id);
		_templates[action_id] = *t;
	}
	menu->setEnabled (!Settings::event_hardware_templates().empty());
}


void
Backend::handle_event_for_learnables (Transport::MidiEvent const& event, EventPort* port)
{
	Learnables::iterator lnext;
	for (Learnables::iterator l = learnables().begin(); l != learnables().end(); l = lnext)
	{
		lnext = l;
		++lnext;

		bool learned = false;
		learned |= (l->second & Keyboard) && (event.type == Transport::MidiEvent::NoteOn || event.type == Transport::MidiEvent::NoteOff);
		learned |= (l->second & Controller) && event.type == Transport::MidiEvent::Controller;
		learned |= (l->second & Pitchbend) && event.type == Transport::MidiEvent::Pitchbend;
		learned |= (l->second & ChannelPressure) && event.type == Transport::MidiEvent::ChannelPressure;
		learned |= (l->second & KeyPressure) && event.type == Transport::MidiEvent::KeyPressure;

		if (learned)
		{
			l->first->learned_connection (l->second, port);
			learnables().erase (l);
		}
	}
}


void
Backend::insert_template (int menu_item_id)
{
	Templates::iterator t = _templates.find (menu_item_id);
	if (t != _templates.end())
	{
		DeviceItem* item = _tree->create_device_item (t->second.name);
		item->load_state (t->second.element);
		item->treeWidget()->clearSelection();
		item->treeWidget()->setCurrentItem (item);
		item->setSelected (true);
	}
}

} // namespace EventBackendImpl

} // namespace Haruhi

