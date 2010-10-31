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

// Haruhi:
#include <haruhi/core/event_buffer.h>
#include <haruhi/config.h>
#include <haruhi/haruhi.h>
#include <haruhi/session.h>

// Local:
#include "transports/alsa_event_transport.h"
#include "event_backend.h"
#include "device_dialog.h"
#include "controller_with_port_item.h"
#include "controller_dialog.h"


namespace Haruhi {

namespace Private = EventBackendPrivate;

EventBackend::EventBackend (Session* session, QString const& client_name, int id, QWidget* parent):
	Unit (0, session, "urn://haruhi.mulabs.org/backend/event-backend/1", "• Event", id, parent),
	_client_name (client_name),
	_tree (0),
	_templates_menu (0)
{
	_transport = new ALSAEventTransport (this);

	//
	// Widgets
	//

	// Ports list:

	_tree = new Private::PortsListView (this, this);

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

	_stack = new QStackedWidget (this);
	_device_dialog = new Private::DeviceDialog (this);
	_controller_dialog = new Private::ControllerDialog (this);
	_stack->addWidget (_device_dialog);
	_stack->addWidget (_controller_dialog);
	_stack->setCurrentWidget (_device_dialog);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::margin, Config::spacing);
	QHBoxLayout* input_buttons_layout = new QHBoxLayout (layout, Config::spacing);
	QHBoxLayout* panels_layout = new QHBoxLayout (layout, Config::spacing);

	QLabel* info = new QLabel ("Devices used in current session. Each device corresponds to external MIDI port.", this);
	info->setMargin (Config::margin);
	layout->addWidget (info);

	panels_layout->addWidget (_tree);
	panels_layout->addWidget (_stack);

	input_buttons_layout->addWidget (_create_device_button);
	input_buttons_layout->addWidget (_create_controller_button);
	input_buttons_layout->addWidget (_destroy_input_button);
	input_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	selection_changed();
	register_unit();
	update_widgets();
}


EventBackend::~EventBackend()
{
	disable();

	_tree->clear();

	if (connected())
		disconnect();
	unregister_unit();

	delete _templates_menu;

	_tree->disconnect();
	_create_device_button->disconnect();
	_create_controller_button->disconnect();
	_destroy_input_button->disconnect();
}


void
EventBackend::process()
{
	// Sync all inputs:
	sync_inputs();
	clear_outputs();

	// Sync transport ports:
	_transport->sync();

	for (InputsMap::iterator h = _inputs.begin(); h != _inputs.end(); ++h)
	{
		EventTransport::Port* transport_port = h->first;
		if (transport_port->buffer().empty())
			continue;
		Private::DeviceWithPortItem* device_item = h->second;
		for (Private::DeviceWithPortItem::Controllers::iterator iii = device_item->controllers()->begin(); iii != device_item->controllers()->end(); ++iii)
		{
			if ((*iii)->ready())
			{
				for (EventTransport::MidiBuffer::iterator m = transport_port->buffer().begin(); m != transport_port->buffer().end(); ++m)
				{
					if ((*iii)->handle_event (*m))
						handle_event_for_learnables (*m, (*iii)->port());
				}
			}
		}
	}
}


void
EventBackend::save_state (QDomElement& element) const
{
	QDomElement inputs = element.ownerDocument().createElement ("inputs");
	_tree->save_state (inputs);
	element.appendChild (inputs);
}


void
EventBackend::load_state (QDomElement const& element)
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
EventBackend::connect()
{
	try {
		_transport->connect (_client_name.toStdString());
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Event backend", QString ("Can't connect to event backend: ") + e.what());
	}
}


void
EventBackend::disconnect()
{
	_transport->disconnect();
}


bool
EventBackend::connected() const
{
	return _transport->connected();
}


void
EventBackend::configure_item (Private::DeviceWithPortItem* item)
{
	_device_dialog->from (item);
	_stack->setCurrentWidget (_device_dialog);
}


void
EventBackend::configure_item (Private::ControllerWithPortItem* item)
{
	_controller_dialog->from (item);
	_stack->setCurrentWidget (_controller_dialog);
}


void
EventBackend::start_learning (Learnable* learnable, EventTypes event_types)
{
	_learnables.insert (std::make_pair (learnable, event_types));
}


void
EventBackend::stop_learning (Learnable* learnable, EventTypes event_types)
{
	_learnables.erase (std::make_pair (learnable, event_types));
}


void
EventBackend::update_widgets()
{
	QTreeWidgetItem* sel = _tree->selected_item();
	_create_controller_button->setEnabled (sel != 0);
	_destroy_input_button->setEnabled (sel != 0);

	// "Destroy device" or "Destroy controller":
	if (sel)
	{
		if (dynamic_cast<Private::DeviceItem*> (sel))
			_destroy_input_button->setText ("Destroy device");
		else if (dynamic_cast<Private::ControllerItem*> (sel))
			_destroy_input_button->setText ("Destroy controller");
		else
			_destroy_input_button->setText ("Destroy");
	}
}


void
EventBackend::selection_changed()
{
	update_widgets();
	configure_selected_input();
}


void
EventBackend::create_device()
{
	QString name = "<unnamed device>";
	QTreeWidgetItem* item = new Private::DeviceWithPortItem (this, _tree, name);
	_tree->setCurrentItem (item);
}


void
EventBackend::create_controller()
{
	QString name = "<unnamed controller>";
	QTreeWidgetItem* sel = _tree->selected_item();
	if (sel != 0)
	{
		Private::DeviceWithPortItem* parent = dynamic_cast<Private::DeviceWithPortItem*> (sel);
		if (parent == 0)
			parent = dynamic_cast<Private::DeviceWithPortItem*> (sel->parent());
		if (parent != 0)
		{
			QTreeWidgetItem* item = new Private::ControllerWithPortItem (parent, name);
			_tree->setCurrentItem (item);
			parent->setExpanded (true);
		}
	}
}


void
EventBackend::context_menu_for_inputs (QPoint const& pos)
{
	Q3PopupMenu* menu = new Q3PopupMenu (this);
	QTreeWidgetItem* item = _tree->itemAt (pos);
	int i;

	if (item != 0)
	{
		if (dynamic_cast<Private::ControllerWithPortItem*> (item) != 0)
		{
			menu->insertItem (Config::Icons16::colorpicker(), "&Learn", this, SLOT (learn_from_midi()));
			menu->insertSeparator();
			menu->insertItem (Config::Icons16::add(), "Add &controller", this, SLOT (create_controller()));
			menu->insertItem (Config::Icons16::remove(), "&Destroy", this, SLOT (destroy_selected_input()));
		}
		else if (dynamic_cast<Private::DeviceWithPortItem*> (item) != 0)
		{
			menu->insertItem (Config::Icons16::add(), "Add &controller", this, SLOT (create_controller()));
			menu->insertSeparator();
			menu->insertItem (Config::Icons16::save(), "&Save as template", this, SLOT (save_selected_input()));
			menu->insertSeparator();
			menu->insertItem (Config::Icons16::add(), "&Add device", this, SLOT (create_device()));
			menu->insertItem (Config::Icons16::remove(), "&Destroy", this, SLOT (destroy_selected_input()));
		}
	}
	else
	{
		menu->insertItem (Config::Icons16::add(), "&Add device", this, SLOT (create_device()));
		i = menu->insertItem (Config::Icons16::remove(), "&Destroy", this, SLOT (destroy_selected_input()));
		menu->setItemEnabled (i, false);
	}
	menu->insertSeparator();
	i = menu->insertItem (Config::Icons16::insert(), "&Insert template", create_templates_menu());
	menu->setItemEnabled (i, !Config::event_hardware_templates().empty());

	menu->exec (QCursor::pos());
	delete menu;
}


void
EventBackend::configure_selected_input()
{
	if (_tree->selected_item())
	{
		Private::DeviceWithPortItem* device_item = dynamic_cast<Private::DeviceWithPortItem*> (_tree->selected_item());
		if (device_item)
			configure_item (device_item);
		else
		{
			Private::ControllerWithPortItem* controller_item = dynamic_cast<Private::ControllerWithPortItem*> (_tree->selected_item());
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
EventBackend::learn_from_midi()
{
	if (_tree->selected_item())
	{
		Private::ControllerWithPortItem* item = dynamic_cast<Private::ControllerWithPortItem*> (_tree->selected_item());
		if (item)
			item->learn();
	}
}


void
EventBackend::destroy_selected_input()
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
EventBackend::save_selected_input()
{
	QTreeWidgetItem* item = _tree->selected_item();
	if (item)
	{
		Private::DeviceWithPortItem* input_item = dynamic_cast<Private::DeviceWithPortItem*> (item);
		if (input_item)
		{
			Config::EventHardwareTemplate tpl (input_item->name());
			input_item->save_state (tpl.element);
			Config::event_hardware_templates().push_back (tpl);
			Config::save_event_hardware_templates();
			QMessageBox::information (this, "Created template", "Created new template \"" + input_item->name() + "\".");
		}
	}
}


Q3PopupMenu*
EventBackend::create_templates_menu()
{
	delete _templates_menu;

	_templates.clear();
	_templates_menu = new Q3PopupMenu (this);
	for (Config::EventHardwareTemplates::iterator t = Config::event_hardware_templates().begin(); t != Config::event_hardware_templates().end(); ++t)
	{
		int i = _templates_menu->insertItem (Config::Icons16::template_(), t->name, this, SLOT (insert_template (int)));
		_templates[i] = *t;
	}
	return _templates_menu;
}


void
EventBackend::handle_event_for_learnables (EventTransport::MidiEvent const& event, Core::EventPort* port)
{
	Learnables::iterator lnext;
	for (Learnables::iterator l = _learnables.begin(); l != _learnables.end(); l = lnext)
	{
		lnext = l;
		++lnext;

		bool learned = false;
		learned |= (l->second & Keyboard) && (event.type == EventTransport::MidiEvent::NoteOn || event.type == EventTransport::MidiEvent::NoteOff);
		learned |= (l->second & Controller) && event.type == EventTransport::MidiEvent::Controller;
		learned |= (l->second & Pitchbend) && event.type == EventTransport::MidiEvent::Pitchbend;
		learned |= (l->second & ChannelPressure) && event.type == EventTransport::MidiEvent::ChannelPressure;
		learned |= (l->second & KeyPressure) && event.type == EventTransport::MidiEvent::KeyPressure;

		if (learned)
		{
			l->first->learned_port (l->second, port);
			_learnables.erase (l);
		}
	}
}


void
EventBackend::insert_template (int menu_item_id)
{
	Templates::iterator t = _templates.find (menu_item_id);
	if (t != _templates.end())
	{
		Private::DeviceWithPortItem* item = new Private::DeviceWithPortItem (this, _tree, t->second.name);
		item->load_state (t->second.element);
		item->treeWidget()->clearSelection();
		item->treeWidget()->setCurrentItem (item);
		item->setSelected (true);
	}
}


} // namespace Haruhi

