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
#include <haruhi/components/event_backend/transports/alsa_event_transport.h>
#include <haruhi/config.h>
#include <haruhi/haruhi.h>
#include <haruhi/session.h>

// Local:
#include "event_backend.h"
#include "external_input_dialog.h"
#include "internal_input_dialog.h"


namespace Haruhi {

namespace Private = EventBackendPrivate;


EventBackend::EventBackend (Session* session, QString const& client_name, int id, QWidget* parent):
	Unit (0, session, "urn://haruhi.mulabs.org/backend/event-backend/1", "• Event", id, parent),
	_session (session),
	_client_name (client_name),
	_inputs_list (0),
	_templates_menu (0)
{
	_transport = new ALSAEventTransport (this);

	//
	// Widgets
	//

	// Ports list:

	_inputs_list = new Private::PortsListView (this, this, "Event inputs");

	QObject::connect (_inputs_list, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_inputs (const QPoint&)));
	QObject::connect (_inputs_list, SIGNAL (itemSelectionChanged()), this, SLOT (selection_changed()));

	_create_external_input_button = new QPushButton (Config::Icons16::add(), "External port", this);
	_create_external_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_create_external_input_button, "Create new external input port");

	_create_internal_input_button = new QPushButton (Config::Icons16::add(), "Internal port", this);
	_create_internal_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_create_internal_input_button, "Create new internal output port connected to external input port");

	_destroy_input_button = new QPushButton (Config::Icons16::remove(), "Destroy", this);
	_destroy_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_destroy_input_button, "Destroy selected external or internal port");

	QObject::connect (_create_external_input_button, SIGNAL (clicked()), this, SLOT (create_external_input()));
	QObject::connect (_create_internal_input_button, SIGNAL (clicked()), this, SLOT (create_internal_input()));
	QObject::connect (_destroy_input_button, SIGNAL (clicked()), this, SLOT (destroy_selected_input()));

	// Right panel (stack):

	_stack = new QStackedWidget (this);
	_external_input_dialog = new Private::ExternalInputDialog (this);
	_internal_input_dialog = new Private::InternalInputDialog (this);
	_stack->addWidget (_external_input_dialog);
	_stack->addWidget (_internal_input_dialog);
	_stack->setCurrentWidget (_external_input_dialog);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::margin, Config::spacing);
	QHBoxLayout* panels_layout = new QHBoxLayout (layout, Config::spacing);
	QHBoxLayout* input_buttons_layout = new QHBoxLayout (layout, Config::spacing);

	panels_layout->addWidget (_inputs_list);
	panels_layout->addWidget (_stack);

	input_buttons_layout->addWidget (_create_external_input_button);
	input_buttons_layout->addWidget (_create_internal_input_button);
	input_buttons_layout->addWidget (_destroy_input_button);
	input_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	selection_changed();
	register_unit();
	connect();
	enable();
}


EventBackend::~EventBackend()
{
	disable();
	_inputs_list->clear();
	if (connected())
		disconnect();

	unregister_unit();

	delete _templates_menu;
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
		Private::ExternalInputItem* external_port_item = h->second;
		for (Private::PortItem::InternalInputs::iterator iii = external_port_item->internal_inputs()->begin(); iii != external_port_item->internal_inputs()->end(); ++iii)
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
	_inputs_list->save_state (inputs);
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
				_inputs_list->load_state (e);
		}
	}
	if (e)
		enable();
}


void
EventBackend::connect()
{
	_transport->connect (_client_name);
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
EventBackend::configure_item (Private::ExternalInputItem* item)
{
	_external_input_dialog->from (item);
	_stack->setCurrentWidget (_external_input_dialog);
}


void
EventBackend::configure_item (Private::InternalInputItem* item)
{
	_internal_input_dialog->from (item);
	_stack->setCurrentWidget (_internal_input_dialog);
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
	QTreeWidgetItem* sel = _inputs_list->selected_item();
	_create_internal_input_button->setEnabled (sel != 0);
	_destroy_input_button->setEnabled (sel != 0);
}


void
EventBackend::selection_changed()
{
	update_widgets();
	configure_selected_input();
}


void
EventBackend::create_external_input()
{
	QString name = "<unnamed>";
	QTreeWidgetItem* item = new Private::ExternalInputItem (_inputs_list, name);
	_inputs_list->setCurrentItem (item);
}


void
EventBackend::create_internal_input()
{
	QString name = "<unnamed>";
	QTreeWidgetItem* sel = _inputs_list->selected_item();
	if (sel != 0)
	{
		Private::ExternalInputItem* parent = dynamic_cast<Private::ExternalInputItem*> (sel);
		if (parent == 0)
			parent = dynamic_cast<Private::ExternalInputItem*> (sel->parent());
		if (parent != 0)
		{
			QTreeWidgetItem* item = new Private::InternalInputItem (parent, name);
			_inputs_list->setCurrentItem (item);
			parent->setExpanded (true);
		}
	}
}


void
EventBackend::context_menu_for_inputs (QPoint const& pos)
{
	Q3PopupMenu* menu = new Q3PopupMenu (this);
	QTreeWidgetItem* item = _inputs_list->itemAt (pos);
	int i;

	if (item != 0)
	{
		if (dynamic_cast<Private::InternalInputItem*> (item) != 0)
		{
			menu->insertItem (Config::Icons16::colorpicker(), "&Learn", this, SLOT (learn_from_midi()));
			menu->insertSeparator();
			menu->insertItem (Config::Icons16::add(), "Create &port", this, SLOT (create_internal_input()));
			menu->insertItem (Config::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_input()));
		}
		else if (dynamic_cast<Private::ExternalInputItem*> (item) != 0)
		{
			menu->insertItem (Config::Icons16::add(), "Create &port", this, SLOT (create_internal_input()));
			menu->insertSeparator();
			menu->insertItem (Config::Icons16::save(), "&Save as template", this, SLOT (save_selected_input()));
			menu->insertSeparator();
			menu->insertItem (Config::Icons16::add(), "&New external port", this, SLOT (create_external_input()));
			menu->insertItem (Config::Icons16::remove(), "&Destroy external port", this, SLOT (destroy_selected_input()));
		}
	}
	else
	{
		menu->insertItem (Config::Icons16::add(), "&New external port", this, SLOT (create_external_input()));
		i = menu->insertItem (Config::Icons16::remove(), "&Destroy external port", this, SLOT (destroy_selected_input()));
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
	if (_inputs_list->selected_item())
	{
		Private::ExternalInputItem* external_input_item = dynamic_cast<Private::ExternalInputItem*> (_inputs_list->selected_item());
		if (external_input_item)
			configure_item (external_input_item);
		else
		{
			Private::InternalInputItem* internal_input_item = dynamic_cast<Private::InternalInputItem*> (_inputs_list->selected_item());
			if (internal_input_item)
				configure_item (internal_input_item);
		}
	}
	else
	{
		_external_input_dialog->clear();
		_internal_input_dialog->clear();
	}
}


void
EventBackend::learn_from_midi()
{
	if (_inputs_list->selected_item())
	{
		Private::InternalInputItem* item = dynamic_cast<Private::InternalInputItem*> (_inputs_list->selected_item());
		if (item)
			item->learn();
	}
}


void
EventBackend::destroy_selected_input()
{
	if (_inputs_list->selected_item())
	{
		QTreeWidgetItem* item = _inputs_list->selected_item();
		if (item->parent())
			item->parent()->takeChild (item->parent()->indexOfChild (item));
		delete item;
	}
}


void
EventBackend::save_selected_input()
{
	QTreeWidgetItem* item = _inputs_list->selected_item();
	if (item)
	{
		Private::ExternalInputItem* input_item = dynamic_cast<Private::ExternalInputItem*> (item);
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
		Private::ExternalInputItem* item = new Private::ExternalInputItem (_inputs_list, t->second.name);
		item->load_state (t->second.element);
		item->treeWidget()->clearSelection();
		item->treeWidget()->setCurrentItem (item);
		item->setSelected (true);
	}
}


} // namespace Haruhi

