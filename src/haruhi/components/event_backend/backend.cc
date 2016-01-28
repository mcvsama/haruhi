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
#include <memory>

// Qt:
#include <QPushButton>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QLayout>
#include <QMenu>
#include <QTextDocument>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/components/devices_manager/device_dialog.h>
#include <haruhi/components/devices_manager/controller_dialog.h>
#include <haruhi/lib/midi.h>
#include <haruhi/graph/event_buffer.h>
#include <haruhi/utility/qdom.h>
#include <haruhi/widgets/styled_background.h>

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
	_client_name (client_name)
{
	_transport = std::make_unique<AlsaTransport> (this);

	// Widgets

	_tree = std::make_unique<Tree> (this, this, &_model);
	_tree->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	QObject::connect (_tree.get(), SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_items (const QPoint&)));
	QObject::connect (_tree.get(), SIGNAL (itemSelectionChanged()), this, SLOT (selection_changed()));

	_create_device_button = std::make_unique<QPushButton> (Resources::Icons16::add(), "Add device", this);
	_create_device_button->setIconSize (Resources::Icons16::haruhi().size());
	_create_device_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_create_device_button->setToolTip ("Add new device and external input port");
	_create_device_button->setFlat (true);

	_create_controller_button = std::make_unique<QPushButton> (Resources::Icons16::add(), "Add controller", this);
	_create_controller_button->setIconSize (Resources::Icons16::haruhi().size());
	_create_controller_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_create_controller_button->setToolTip ("Add new controller and internal output port");
	_create_controller_button->setFlat (true);

	_destroy_input_button = std::make_unique<QPushButton> (Resources::Icons16::remove(), "Destroy", this);
	_destroy_input_button->setIconSize (Resources::Icons16::haruhi().size());
	_destroy_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_destroy_input_button->setToolTip ("Destroy selected device or controller");
	_destroy_input_button->setFlat (true);

	_insert_template_button = std::make_unique<QPushButton> (Resources::Icons16::insert(), "Insert template", this);
	_insert_template_button->setIconSize (Resources::Icons16::haruhi().size());
	_insert_template_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_insert_template_button->setToolTip ("Insert device previously saved in 'Device templates'");
	_insert_template_button->setFlat (true);

	QObject::connect (_create_device_button.get(), SIGNAL (clicked()), _tree.get(), SLOT (create_device()));
	QObject::connect (_create_controller_button.get(), SIGNAL (clicked()), _tree.get(), SLOT (create_controller()));
	QObject::connect (_destroy_input_button.get(), SIGNAL (clicked()), _tree.get(), SLOT (destroy_selected_item()));

	// Configuration panel (stack):

	_stack = std::make_unique<QStackedWidget> (this);
	_stack->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Maximum);
	_device_dialog = std::make_unique<DeviceWithPortDialog> (this);
	_controller_dialog = std::make_unique<ControllerWithPortDialog> (this);
	_stack->addWidget (_device_dialog.get());
	_stack->addWidget (_controller_dialog.get());
	_stack->setCurrentWidget (_device_dialog.get());

	auto info = new QLabel ("Each device corresponds to externally visible MIDI port.", this);
	info->setMargin (Config::margin());

	// Layouts:

	auto input_buttons_layout = new QHBoxLayout();
	input_buttons_layout->setSpacing (Config::spacing());
	input_buttons_layout->addWidget (_insert_template_button.get());
	input_buttons_layout->addWidget (_create_device_button.get());
	input_buttons_layout->addWidget (_create_controller_button.get());
	input_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	input_buttons_layout->addWidget (_destroy_input_button.get());

	auto panels_layout = new QVBoxLayout();
	panels_layout->setSpacing (Config::spacing());
	panels_layout->addWidget (_tree.get());
	panels_layout->addWidget (_stack.get());

	auto layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (new StyledBackground (new QLabel ("ALSA MIDI ports"), this));
	layout->setSpacing (Config::spacing());
	layout->addLayout (input_buttons_layout);
	layout->addLayout (panels_layout);
	layout->addWidget (info);

	selection_changed();
	update_widgets();
	update_templates_menu();
}


Backend::~Backend()
{
	_tree->disconnect();
	_create_device_button->disconnect();
	_create_controller_button->disconnect();
	_destroy_input_button->disconnect();
}


bool
Backend::connected() const
{
	return _transport->connected();
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
	for (auto& h: _inputs)
	{
		auto transport_port = h.first;
		auto device_item = h.second;

		// For each event that comes from transport:
		for (MIDI::Event& m: transport_port->buffer())
		{
			// For each Controller:
			for (auto* c: *device_item->controllers())
			{
				if (c->ready())
				{
					if (c->handle_event (m, *device_item))
						handle_event_for_learnables (m, c->port());
					on_event (m);
				}
			}
		}

		// For each Controller:
		for (auto* c: *device_item->controllers())
			c->generate_smoothing_events();
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
	for (QDomElement& e: element)
	{
		if (e.tagName() == "inputs")
			_model.load_state (e);
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
		QApplication::postEvent (this, new StateChange (true));
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Event backend", "Can't connect to event backend: " + QString (e.what()).toHtmlEscaped());
	}
}


void
Backend::disconnect()
{
	disable();
	_transport->disconnect();
	QApplication::postEvent (this, new StateChange (false));
}


void
Backend::update_widgets()
{
	auto sel = _tree->selected_item();
	_create_controller_button->setEnabled (sel != 0);
	_destroy_input_button->setEnabled (sel != 0);
}


void
Backend::save_selected_item_as_template()
{
	auto item = _tree->selected_item();
	if (item)
	{
		auto device_item = dynamic_cast<DeviceWithPortItem*> (item);
		if (device_item)
			device_saved_as_template (*device_item->device());
	}
}


void
Backend::context_menu_for_items (QPoint const& pos)
{
	auto menu = std::make_unique<QMenu> (this);
	auto item = _tree->itemAt (pos);

	if (item != 0)
	{
		auto cwp_item = dynamic_cast<ControllerWithPortItem*> (item);
		if (cwp_item != 0)
		{
			menu->addAction (Resources::Icons16::colorpicker(), cwp_item->learning() ? "Stop &learning" : "&Learn", this, SLOT (learn_from_midi()));
			menu->addSeparator();
			menu->addAction (Resources::Icons16::add(), "Add &controller", _tree.get(), SLOT (create_controller()));
			menu->addAction (Resources::Icons16::remove(), "&Destroy", _tree.get(), SLOT (destroy_selected_item()));
		}
		else if (dynamic_cast<DeviceItem*> (item) != 0)
		{
			menu->addAction (Resources::Icons16::add(), "Add &controller", _tree.get(), SLOT (create_controller()));
			menu->addSeparator();
			if (device_saved_as_template.connections_number() > 0)
			{
				menu->addAction (Resources::Icons16::save(), "&Save as template", this, SLOT (save_selected_item_as_template()));
				menu->addSeparator();
			}
			menu->addAction (Resources::Icons16::add(), "&Add device", _tree.get(), SLOT (create_device()));
			menu->addAction (Resources::Icons16::remove(), "&Destroy", _tree.get(), SLOT (destroy_selected_item()));
		}
	}
	else
	{
		menu->addAction (Resources::Icons16::add(), "&Add device", _tree.get(), SLOT (create_device()));
		auto a = menu->addAction (Resources::Icons16::remove(), "&Destroy", _tree.get(), SLOT (destroy_selected_item()));
		a->setEnabled (false);
	}
	menu->addSeparator();
	menu->addMenu (_templates_menu.get());

	menu->exec (QCursor::pos());
}


void
Backend::update_templates_menu()
{
	_templates_menu = std::make_unique<QMenu>();
	_insert_template_signal_mapper = std::make_unique<QSignalMapper> (this);
	QObject::connect (_insert_template_signal_mapper.get(), SIGNAL (mapped (int)), this, SLOT (add_template (int)));

	int action_id = 0;
	_templates.clear();
	DevicesManager::Model& dm_model = Haruhi::haruhi()->devices_manager_settings()->model();
	for (DevicesManager::Device& d: dm_model.devices())
	{
		action_id += 1;
		auto a = _templates_menu->addAction (Resources::Icons16::keyboard(), d.name(), _insert_template_signal_mapper.get(), SLOT (map()));
		_insert_template_signal_mapper->setMapping (a, action_id);
		_templates.insert (std::make_pair (action_id, d));
	}

	_templates_menu->setTitle ("&Insert template");
	_templates_menu->setIcon (Resources::Icons16::insert());
	_templates_menu->setEnabled (!dm_model.devices().empty());

	_insert_template_button->setMenu (_templates_menu.get());
	_insert_template_button->setEnabled (!dm_model.devices().empty());
}


void
Backend::handle_event_for_learnables (MIDI::Event const& event, EventPort* port)
{
	Learnables::iterator lnext;
	for (auto l = learnables().begin(); l != learnables().end(); l = lnext)
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
	_stack->setCurrentWidget (_device_dialog.get());
}


void
Backend::configure_item (ControllerItem* item)
{
	_controller_dialog->from (item);
	_stack->setCurrentWidget (_controller_dialog.get());
}


void
Backend::configure_selected_item()
{
	if (_tree->selected_item())
	{
		auto device_item = dynamic_cast<DeviceItem*> (_tree->selected_item());
		if (device_item)
			configure_item (device_item);
		else
		{
			auto controller_item = dynamic_cast<ControllerItem*> (_tree->selected_item());
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
		if (_transport->learning_possible())
		{
			auto item = dynamic_cast<ControllerWithPortItem*> (_tree->selected_item());
			if (item)
				item->learn();
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
Backend::add_template (int menu_item_id)
{
	Templates::iterator t = _templates.find (menu_item_id);
	if (t != _templates.end())
		add_device (t->second);
}


void
Backend::customEvent (QEvent* event)
{
	auto state_change = dynamic_cast<StateChange*> (event);
	if (state_change)
		on_state_change (state_change->online);
}

} // namespace EventBackendImpl

} // namespace Haruhi

