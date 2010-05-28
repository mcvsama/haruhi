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
#include <cstdlib>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtGui/QToolTip>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/session.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "transports/jack_audio_transport.h"
#include "audio_backend.h"


// TODO handle dummy_timer and faked sample_rate+buffer_size when transport disconnects.
namespace Haruhi {

namespace Private = AudioBackendPrivate;


AudioBackend::AudioBackend (Session* session, QString const& client_name, int id, QWidget* parent):
	Unit (0, session, "urn://haruhi.mulabs.org/backend/jack-audio-backend/1", "• Audio", id, parent),
	_client_name (client_name)
{
	_transport = new JackAudioTransport (this);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::margin, Config::spacing);
	QHBoxLayout* top_layout = new QHBoxLayout (layout, Config::spacing);
	QHBoxLayout* lists_layout = new QHBoxLayout (layout, Config::spacing);
	QVBoxLayout* inputs_layout = new QVBoxLayout (lists_layout, Config::spacing);
	QVBoxLayout* outputs_layout = new QVBoxLayout (lists_layout, Config::spacing);

	_disconnect_button = new QPushButton (Config::Icons16::connect(), "Disconnect from JACK", this);
	_disconnect_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QObject::connect (_disconnect_button, SIGNAL (clicked()), this, SLOT (disconnect()));

	_reconnect_button = new QPushButton (Config::Icons16::connect(), "Reconnect to JACK", this);
	_reconnect_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QObject::connect (_reconnect_button, SIGNAL (clicked()), this, SLOT (connect()));

	top_layout->addWidget (_disconnect_button);
	top_layout->addWidget (_reconnect_button);
	top_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

		_inputs_list = new Private::PortsListView (this, this, "Audio inputs");

	inputs_layout->addWidget (_inputs_list);

	QObject::connect (_inputs_list, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_inputs (const QPoint&)));
	QObject::connect (_inputs_list, SIGNAL (itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT (double_click_on_inputs (QTreeWidgetItem*, int)));
	QObject::connect (_inputs_list, SIGNAL (itemSelectionChanged()), this, SLOT (update_widgets()));

	QHBoxLayout* input_buttons_layout = new QHBoxLayout (inputs_layout, Config::spacing);

		_create_input_button = new QPushButton (Config::Icons16::add(), "New input", this);
		_create_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		QToolTip::add (_create_input_button, "Create new input port");

		_destroy_input_button = new QPushButton (Config::Icons16::remove(), "Destroy", this);
		_destroy_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		QToolTip::add (_destroy_input_button, "Destroy selected port");

	input_buttons_layout->addWidget (_create_input_button);
	input_buttons_layout->addWidget (_destroy_input_button);
	input_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	QObject::connect (_create_input_button, SIGNAL (clicked()), this, SLOT (create_input()));
	QObject::connect (_destroy_input_button, SIGNAL (clicked()), this, SLOT (destroy_selected_input()));

		_outputs_list = new Private::PortsListView (this, this, "Audio outputs");

	outputs_layout->addWidget (_outputs_list);

	QObject::connect (_outputs_list, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_outputs (const QPoint&)));
	QObject::connect (_outputs_list, SIGNAL (itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT (double_click_on_outputs (QTreeWidgetItem*, int)));
	QObject::connect (_outputs_list, SIGNAL (itemSelectionChanged()), this, SLOT (update_widgets()));

	QHBoxLayout* output_buttons_layout = new QHBoxLayout (outputs_layout, Config::spacing);

		_create_output_button = new QPushButton (Config::Icons16::add(), "New output", this);
		_create_output_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		QToolTip::add (_create_output_button, "Create new output port");

		_destroy_output_button = new QPushButton (Config::Icons16::remove(), "Destroy", this);
		_destroy_output_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		QToolTip::add (_destroy_output_button, "Destroy selected port");

	output_buttons_layout->addWidget (_create_output_button);
	output_buttons_layout->addWidget (_destroy_output_button);
	output_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	QObject::connect (_create_output_button, SIGNAL (clicked()), this, SLOT (create_output()));
	QObject::connect (_destroy_output_button, SIGNAL (clicked()), this, SLOT (destroy_selected_output()));

	register_unit();

	_master_volume_port = new Core::EventPort (this, "Master Volume", Core::Port::Input);
	_panic_port = new Core::EventPort (this, "Panic", Core::Port::Input);

	// Graph ticks emulator:
	_dummy_timer = new QTimer (this);
	QObject::connect (_dummy_timer, SIGNAL (timeout()), this, SLOT (dummy_round()));
	_dummy_timer->start (33);

	update_widgets();
}


AudioBackend::~AudioBackend()
{
	_dummy_timer->stop();

	disable();

	// Deallocate all ports:
	_inputs_list->clear();
	_outputs_list->clear();

	if (connected())
		disconnect();

	delete _master_volume_port;
	delete _panic_port;

	unregister_unit();
}


void
AudioBackend::enable()
{
	Unit::enable();
	_transport->activate();
}


void
AudioBackend::disable()
{
	_transport->deactivate();
	Unit::disable();
}


void
AudioBackend::transfer()
{
	session()->engine()->wait_for_data();

	_ports_lock.lock();
	// Transport -> Haruhi:
	for (InputsMap::iterator p = _inputs.begin(); p != _inputs.end(); ++p)
	{
		Core::Sample* src_buffer = p->first->buffer();
		Core::AudioBuffer* dst_buffer = p->second->port()->audio_buffer();
		if (src_buffer)
			memcpy (dst_buffer->begin(), src_buffer, sizeof (Core::Sample) * dst_buffer->size());
		else
			dst_buffer->clear();
	}

	// Use Master Volume control to adjust volume of outputs:
	// FIXME not-secure, use exported int value (controller_proxy->value()):
	DialControl* master_volume = session()->meter_panel()->master_volume();
	Core::Sample v = std::pow (master_volume->value() / static_cast<float> (Session::MeterPanel::ZeroVolume), M_E);
	for (OutputsMap::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
		p->second->attenuate (v);

	// Haruhi -> Transport:
	for (OutputsMap::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
	{
		Core::AudioBuffer* src_buffer = p->second->port()->audio_buffer();
		Core::Sample* dst_buffer = p->first->buffer();
		if (dst_buffer)
		{
			if (p->second->port()->back_connections().empty())
				memset (dst_buffer, 0, sizeof (Core::Sample) * src_buffer->size());
			else
				memcpy (dst_buffer, src_buffer->begin(), sizeof (Core::Sample) * src_buffer->size());
		}
	}

	update_level_meter();
	_ports_lock.unlock();

	// Tell engine to continue processing:
	session()->engine()->continue_processing();
}


void
AudioBackend::process()
{
	if (graph()->dummy())
		return;

	sync_inputs();
}


void
AudioBackend::save_state (QDomElement& element) const
{
	QDomElement inputs = element.ownerDocument().createElement ("inputs");
	QDomElement outputs = element.ownerDocument().createElement ("outputs");

	_inputs_list->save_state (inputs);
	_outputs_list->save_state (outputs);

	element.appendChild (inputs);
	element.appendChild (outputs);
}


void
AudioBackend::load_state (QDomElement const& element)
{
	bool active = _transport->active();
	if (active)
		_transport->deactivate();

	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "inputs")
				_inputs_list->load_state (e);
			else if (e.tagName() == "outputs")
				_outputs_list->load_state (e);
		}
	}

	if (active)
		_transport->activate();
}


void
AudioBackend::connect()
{
	try {
		_dummy_timer->stop();
		_transport->connect (_client_name.toStdString());
		_transport->activate();
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Audio backend", QString ("Can't connect to audio backend: ") + e.what());
		_dummy_timer->start();
	}
	update_widgets();
}


void
AudioBackend::disconnect()
{
	_transport->deactivate();
	_transport->disconnect();
	_dummy_timer->start();
	update_widgets();
}


bool
AudioBackend::connected() const
{
	return _transport->connected();
}


void
AudioBackend::notify_disconnected()
{
	QApplication::postEvent (this, new OfflineNotification());
}


void
AudioBackend::update_widgets()
{
	_disconnect_button->setEnabled (connected());
	_reconnect_button->setEnabled (!connected());

	QTreeWidgetItem* sel = _inputs_list->selected_item();
	_destroy_input_button->setEnabled (sel != 0);

	sel = _outputs_list->selected_item();
	_destroy_output_button->setEnabled (sel != 0);
}


void
AudioBackend::create_input()
{
	Private::InputDialog* dialog = new Private::InputDialog (this, this);
	if (dialog->exec() == Private::InputDialog::Accepted)
	{
		Private::InputItem* item = new Private::InputItem (_inputs_list, dialog->name());
		dialog->apply (item);
		_inputs_list->setCurrentItem (item);
	}
}


void
AudioBackend::create_input (QString const& name)
{
	Private::InputItem* item = new Private::InputItem (_inputs_list, name);
	_inputs_list->setCurrentItem (item);
}


void
AudioBackend::create_output()
{
	Private::OutputDialog* dialog = new Private::OutputDialog (this, this);
	if (dialog->exec() == Private::OutputDialog::Accepted)
	{
		Private::OutputItem* item = new Private::OutputItem (_outputs_list, dialog->name());
		dialog->apply (item);
		_outputs_list->setCurrentItem (item);
	}
}


void
AudioBackend::create_output (QString const& name)
{
	Private::OutputItem* item = new Private::OutputItem (_outputs_list, name);
	_outputs_list->setCurrentItem (item);
}


void
AudioBackend::context_menu_for_inputs (QPoint const& pos)
{
	Q3PopupMenu* menu = new Q3PopupMenu (this);
	QTreeWidgetItem* item = _inputs_list->itemAt (pos);
	int i;

	if (item != 0)
	{
		if (dynamic_cast<Private::InputItem*> (item) != 0)
		{
			i = menu->insertItem (Config::Icons16::rename(), "&Rename", this, SLOT (rename_selected_input()));
			menu->insertSeparator();
			i = menu->insertItem (Config::Icons16::add(), "&New port", this, SLOT (create_input()));
			menu->setItemEnabled (i, false);
			i = menu->insertItem (Config::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_input()));
		}
	}
	else
	{
		i = menu->insertItem (Config::Icons16::add(), "&New port", this, SLOT (create_input()));
		i = menu->insertItem (Config::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_input()));
		menu->setItemEnabled (i, false);
	}

	menu->exec (QCursor::pos());
	delete menu;
}


void
AudioBackend::context_menu_for_outputs (QPoint const& pos)
{
	Q3PopupMenu* menu = new Q3PopupMenu (this);
	QTreeWidgetItem* item = _outputs_list->itemAt (pos);
	int i;

	if (item != 0)
	{
		if (dynamic_cast<Private::OutputItem*> (item) != 0)
		{
			i = menu->insertItem (Config::Icons16::rename(), "&Rename", this, SLOT (rename_selected_output()));
			menu->insertSeparator();
			i = menu->insertItem (Config::Icons16::add(), "&New port", this, SLOT (create_output()));
			menu->setItemEnabled (i, false);
			i = menu->insertItem (Config::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_output()));
		}
	}
	else
	{
		i = menu->insertItem (Config::Icons16::add(), "&New port", this, SLOT (create_output()));
		i = menu->insertItem (Config::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_output()));
		menu->setItemEnabled (i, false);
	}

	menu->exec (QCursor::pos());
	delete menu;
}


void
AudioBackend::double_click_on_inputs (QTreeWidgetItem* item, int)
{
	if (item != 0)
	{
		item->treeWidget()->setCurrentItem (item);
		configure_selected_input();
	}
}


void
AudioBackend::double_click_on_outputs (QTreeWidgetItem* item, int)
{
	if (item != 0)
	{
		item->treeWidget()->setCurrentItem (item);
		configure_selected_output();
	}
}


void
AudioBackend::configure_selected_input()
{
	if (_inputs_list->selected_item())
	{
		Private::PortItem* item = dynamic_cast<Private::PortItem*> (_inputs_list->selected_item());
		if (item)
			item->configure();
	}
}


void
AudioBackend::configure_selected_output()
{
	if (_outputs_list->selected_item())
	{
		Private::PortItem* item = dynamic_cast<Private::PortItem*> (_outputs_list->selected_item());
		if (item)
			item->configure();
	}
}


void
AudioBackend::rename_selected_input()
{
	QTreeWidgetItem* sel = _inputs_list->selected_item();
	if (sel != 0)
		configure_selected_input();
}


void
AudioBackend::rename_selected_output()
{
	QTreeWidgetItem* sel = _outputs_list->selected_item();
	if (sel != 0)
		configure_selected_output();
}


void
AudioBackend::destroy_selected_input()
{
	if (_inputs_list->selected_item())
	{
		QTreeWidgetItem* item = _inputs_list->selected_item();
		_inputs_list->invisibleRootItem()->takeChild (_inputs_list->invisibleRootItem()->indexOfChild (item));
		delete item;
	}
}


void
AudioBackend::destroy_selected_output()
{
	if (_outputs_list->selected_item())
	{
		QTreeWidgetItem* item = _outputs_list->selected_item();
		_outputs_list->invisibleRootItem()->takeChild (_inputs_list->invisibleRootItem()->indexOfChild (item));
		delete item;
	}
}


void
AudioBackend::dummy_round()
{
	session()->engine()->continue_processing();
}


void
AudioBackend::update_level_meter()
{
	// Level meter:
	std::vector<Private::OutputItem*> ovec;
	for (OutputsMap::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
		ovec.push_back (p->second);
	std::sort (ovec.begin(), ovec.end(), Private::PortItem::CompareByPortName());
	for (unsigned int i = 0; i < std::min (ovec.size(), static_cast<std::vector<Private::OutputItem*>::size_type> (2u)); ++i)
	{
		Core::AudioBuffer* abuf = ovec[i]->port()->audio_buffer();
		session()->meter_panel()->level_meters_group()->meter (i)->process (abuf->begin(), abuf->end());
	}
}


void
AudioBackend::customEvent (QEvent* event)
{
	OfflineNotification* offline_notification = dynamic_cast<OfflineNotification*> (event);
	if (offline_notification)
	{
		_dummy_timer->start();
		update_widgets();
		// Show message to user:
		QMessageBox::warning (this, "Audio backend", "Audio transport disconnected. :[\nUse \"Reconnect\" button on Audio backend tab (or press C-j) to reconnect.");
	}
}


void
AudioBackend::graph_updated()
{
	Unit::graph_updated();
	// Update smoothers for all OutputItems:
	for (OutputsMap::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
		p->second->graph_updated();
}

} // namespace Haruhi

