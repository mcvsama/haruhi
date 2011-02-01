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

// Qt:
#include <QtGui/QPushButton>
#include <QtGui/QToolTip>
#include <QtGui/QMessageBox>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QMenu>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/components/devices_manager/device_dialog.h>
#include <haruhi/components/devices_manager/controller_dialog.h>
#include <haruhi/lib/midi.h>
#include <haruhi/graph/event_buffer.h>

// Local:
#include "transports/alsa_transport.h"
#include "backend.h"
#include "device_with_port_dialog.h"
#include "controller_with_port_dialog.h"
#include "controller_with_port_item.h"
#include "tree.h"


namespace Haruhi {

namespace EventBackendImpl {

Backend::Backend (QString const& client_name, QWidget* parent):
	QWidget (parent),
	EventBackend ("╸Devices╺"),
	_client_name (client_name),
	_insert_template_signal_mapper (0),
	_templates_menu (0)
{
	_transport = new AlsaTransport (this);

	//
	// Widgets
	//

	_tree = new Tree (this, this, &_model);
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

	_insert_template_button = new QPushButton (Resources::Icons16::insert(), "Insert template", this);
	_insert_template_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_insert_template_button, "Insert device previously saved in Devices manager");

	QObject::connect (_create_device_button, SIGNAL (clicked()), _tree, SLOT (create_device()));
	QObject::connect (_create_controller_button, SIGNAL (clicked()), _tree, SLOT (create_controller()));
	QObject::connect (_destroy_input_button, SIGNAL (clicked()), _tree, SLOT (destroy_selected_item()));

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

	input_buttons_layout->addWidget (_insert_template_button);
	input_buttons_layout->addWidget (_create_device_button);
	input_buttons_layout->addWidget (_create_controller_button);
	input_buttons_layout->addWidget (_destroy_input_button);
	input_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	selection_changed();
	update_widgets();
	update_templates_menu();
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
Backend::add_device (DevicesManager::Device const& device)
{
	_model.devices().push_back (device);
	_model.changed();
}


void
Backend::devices_manager_updated()
{
	update_templates_menu();
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

	// For each Device:
	for (InputsMap::iterator h = _inputs.begin(); h != _inputs.end(); ++h)
	{
		Transport::Port* transport_port = h->first;
		DeviceWithPortItem* device_item = h->second;
		// For each Controller:
		for (DeviceWithPortItem::Controllers::iterator c = device_item->controllers()->begin(); c != device_item->controllers()->end(); ++c)
		{
			if ((*c)->ready())
			{
				// For each event that comes from transport:
				for (Transport::MidiBuffer::iterator m = transport_port->buffer().begin(); m != transport_port->buffer().end(); ++m)
				{
					if ((*c)->handle_event (*m))
						handle_event_for_learnables (*m, (*c)->port());
					on_event (*m);
				}
				(*c)->generate_smoothing_events();
			}
		}
	}
}


void
Backend::save_state (QDomElement& element) const
{
	QDomElement inputs = element.ownerDocument().createElement ("inputs");
	_model.save_state (inputs);
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
				_model.load_state (e);
		}
	}
	if (e)
		enable();
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
Backend::save_selected_item_as_template()
{
	QTreeWidgetItem* item = _tree->selected_item();
	if (item)
	{
		DeviceWithPortItem* device_item = dynamic_cast<DeviceWithPortItem*> (item);
		if (device_item)
			device_saved_as_template (*device_item->device());
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
		ControllerWithPortItem* cwp_item = dynamic_cast<ControllerWithPortItem*> (item);
		if (cwp_item != 0)
		{
			menu->addAction (Resources::Icons16::colorpicker(), cwp_item->learning() ? "Stop &learning" : "&Learn", this, SLOT (learn_from_midi()));
			menu->addSeparator();
			menu->addAction (Resources::Icons16::add(), "Add &controller", _tree, SLOT (create_controller()));
			menu->addAction (Resources::Icons16::remove(), "&Destroy", _tree, SLOT (destroy_selected_item()));
		}
		else if (dynamic_cast<DeviceItem*> (item) != 0)
		{
			menu->addAction (Resources::Icons16::add(), "Add &controller", _tree, SLOT (create_controller()));
			menu->addSeparator();
			if (device_saved_as_template.connections_number() > 0)
			{
				menu->addAction (Resources::Icons16::save(), "&Save as template", this, SLOT (save_selected_item_as_template()));
				menu->addSeparator();
			}
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
	menu->addSeparator();
	menu->addMenu (_templates_menu);

	menu->exec (QCursor::pos());
	delete menu;
}


void
Backend::update_templates_menu()
{
	delete _templates_menu;

	_templates_menu = new QMenu();
	if (_insert_template_signal_mapper)
		delete _insert_template_signal_mapper;
	_insert_template_signal_mapper = new QSignalMapper (this);
	QObject::connect (_insert_template_signal_mapper, SIGNAL (mapped (int)), this, SLOT (add_template (int)));

	int action_id = 0;
	_templates.clear();
	DevicesManager::Model& dm_model = Haruhi::haruhi()->devices_manager_settings()->model();
	for (DevicesManager::Model::Devices::iterator d = dm_model.devices().begin(); d != dm_model.devices().end(); ++d)
	{
		action_id += 1;
		QAction* a = _templates_menu->addAction (Resources::Icons16::template_(), d->name(), _insert_template_signal_mapper, SLOT (map()));
		_insert_template_signal_mapper->setMapping (a, action_id);
		_templates.insert (std::make_pair (action_id, *d));
	}

	_templates_menu->setTitle ("&Insert template");
	_templates_menu->setIcon (Resources::Icons16::insert());
	_templates_menu->setEnabled (!dm_model.devices().empty());

	_insert_template_button->setMenu (_templates_menu);
	_insert_template_button->setEnabled (!dm_model.devices().empty());
}


void
Backend::handle_event_for_learnables (MIDI::Event const& event, EventPort* port)
{
	Learnables::iterator lnext;
	for (Learnables::iterator l = learnables().begin(); l != learnables().end(); l = lnext)
	{
		lnext = l;
		++lnext;

		bool learned = false;
		learned |= (l->second & Keyboard) && (event.type == MIDI::Event::NoteOn || event.type == MIDI::Event::NoteOff);
		learned |= (l->second & Controller) && event.type == MIDI::Event::Controller;
		learned |= (l->second & Pitchbend) && event.type == MIDI::Event::Pitchbend;
		learned |= (l->second & ChannelPressure) && event.type == MIDI::Event::ChannelPressure;
		learned |= (l->second & KeyPressure) && event.type == MIDI::Event::KeyPressure;

		if (learned)
		{
			l->first->learned_connection (l->second, port);
			learnables().erase (l);
		}
	}
}


void
Backend::selection_changed()
{
	update_widgets();
	configure_selected_item();
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
Backend::add_template (int menu_item_id)
{
	Templates::iterator t = _templates.find (menu_item_id);
	if (t != _templates.end())
		add_device (t->second);
}

} // namespace EventBackendImpl

} // namespace Haruhi

