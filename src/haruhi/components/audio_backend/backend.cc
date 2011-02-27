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
#include <QtGui/QMenu>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "transports/jack_transport.h"
#include "backend.h"


namespace Haruhi {

namespace AudioBackendImpl {

Backend::Backend (QString const& client_name, QWidget* parent):
	QWidget (parent),
	AudioBackend ("╸Audio╺"),
	_client_name (client_name)
{
	_master_volume_port = new EventPort (this, "Master Volume", Port::Input);
	_panic_port = new EventPort (this, "Panic", Port::Input);

	_transport = new JackTransport (this);

	_disconnect_button = new QPushButton (Resources::Icons16::connect(), "Disconnect from JACK", this);
	_disconnect_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QObject::connect (_disconnect_button, SIGNAL (clicked()), this, SLOT (disconnect()));

	_reconnect_button = new QPushButton (Resources::Icons16::connect(), "Reconnect to JACK", this);
	_reconnect_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QObject::connect (_reconnect_button, SIGNAL (clicked()), this, SLOT (connect()));

	_inputs_list = new Tree (this, this, "Audio inputs");
	QObject::connect (_inputs_list, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_inputs (const QPoint&)));
	QObject::connect (_inputs_list, SIGNAL (itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT (double_click_on_inputs (QTreeWidgetItem*, int)));
	QObject::connect (_inputs_list, SIGNAL (itemSelectionChanged()), this, SLOT (update_widgets()));

	_create_input_button = new QPushButton (Resources::Icons16::add(), "New input", this);
	_create_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_create_input_button, "Create new input port");
	QObject::connect (_create_input_button, SIGNAL (clicked()), this, SLOT (create_input()));

	_destroy_input_button = new QPushButton (Resources::Icons16::remove(), "Destroy", this);
	_destroy_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_destroy_input_button, "Destroy selected port");
	QObject::connect (_destroy_input_button, SIGNAL (clicked()), this, SLOT (destroy_selected_input()));

	_outputs_list = new Tree (this, this, "Audio outputs");
	QObject::connect (_outputs_list, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_outputs (const QPoint&)));
	QObject::connect (_outputs_list, SIGNAL (itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT (double_click_on_outputs (QTreeWidgetItem*, int)));
	QObject::connect (_outputs_list, SIGNAL (itemSelectionChanged()), this, SLOT (update_widgets()));

	_create_output_button = new QPushButton (Resources::Icons16::add(), "New output", this);
	_create_output_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_create_output_button, "Create new output port");
	QObject::connect (_create_output_button, SIGNAL (clicked()), this, SLOT (create_output()));

	_destroy_output_button = new QPushButton (Resources::Icons16::remove(), "Destroy", this);
	_destroy_output_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QToolTip::add (_destroy_output_button, "Destroy selected port");
	QObject::connect (_destroy_output_button, SIGNAL (clicked()), this, SLOT (destroy_selected_output()));

	// Layouts:

	QHBoxLayout* input_buttons_layout = new QHBoxLayout();
	input_buttons_layout->setSpacing (Config::Spacing);
	input_buttons_layout->addWidget (_create_input_button);
	input_buttons_layout->addWidget (_destroy_input_button);
	input_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	QVBoxLayout* inputs_layout = new QVBoxLayout();
	inputs_layout->setSpacing (Config::Spacing);
	inputs_layout->addWidget (_inputs_list);
	inputs_layout->addLayout (input_buttons_layout);

	QHBoxLayout* output_buttons_layout = new QHBoxLayout();
	output_buttons_layout->setSpacing (Config::Spacing);
	output_buttons_layout->addWidget (_create_output_button);
	output_buttons_layout->addWidget (_destroy_output_button);
	output_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	QVBoxLayout* outputs_layout = new QVBoxLayout();
	outputs_layout->setSpacing (Config::Spacing);
	outputs_layout->addWidget (_outputs_list);
	outputs_layout->addLayout (output_buttons_layout);

	QHBoxLayout* top_layout = new QHBoxLayout();
	top_layout->setSpacing (Config::Spacing);
	top_layout->addWidget (_disconnect_button);
	top_layout->addWidget (_reconnect_button);
	top_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	QHBoxLayout* lists_layout = new QHBoxLayout();
	lists_layout->setSpacing (Config::Spacing);
	lists_layout->addLayout (inputs_layout);
	lists_layout->addLayout (outputs_layout);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::Margin);
	layout->setSpacing (Config::Spacing);
	layout->addLayout (top_layout);
	layout->addLayout (lists_layout);

	update_widgets();
}


Backend::~Backend()
{
	delete _master_volume_port;
	delete _panic_port;
}


bool
Backend::connected() const
{
	return _transport->connected();
}


void
Backend::enable()
{
	Unit::enable();
	_transport->activate();
}


void
Backend::disable()
{
	_transport->deactivate();
	Unit::disable();
}


void
Backend::registered()
{
	graph_updated();
	connect();
}


void
Backend::unregistered()
{
	if (connected())
		disconnect();

	// Deallocate all ports:
	_inputs_list->clear();
	_outputs_list->clear();
}


void
Backend::process()
{
	if (graph()->dummy())
		return;

	sync_inputs();
}


void
Backend::data_ready()
{
	if (!_transport->connected() || !_transport->active())
	{
		dummy_round();
		return;
	}

	_ports_lock.lock();
	// Use Master Volume control to adjust volume of outputs:
	_master_volume_smoother.fill (_master_volume_smoother_buffer.begin(),
								  _master_volume_smoother_buffer.end(),
								  master_volume());
	for (OutputsMap::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
		if (p->second->ready())
			p->second->port()->audio_buffer()->attenuate (&_master_volume_smoother_buffer);

	// Copy data from graph to transport (output):
	_transport->lock_ports();
	for (OutputsMap::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
	{
		if (!p->second->ready())
			continue;
		if (p->second->port()->back_connections().empty())
			p->first->buffer()->clear();
		else
			p->first->buffer()->fill (p->second->port()->audio_buffer());
	}
	_transport->unlock_ports();
	_ports_lock.unlock();

	_transport->data_ready();

	// Copy data from transport to graph (input):
	_ports_lock.lock();
	_transport->lock_ports();
	for (InputsMap::iterator p = _inputs.begin(); p != _inputs.end(); ++p)
		if (p->second->ready())
			p->first->buffer()->fill (p->second->port()->buffer());
	_transport->unlock_ports();
	_ports_lock.unlock();
}


void
Backend::peak_levels (LevelsMap& levels)
{
	levels.clear();

	_ports_lock.lock();
	for (OutputsMap::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
	{
		Sample register max = 0;
		AudioPort* port = p->second->port();
		AudioBuffer* buf = port->audio_buffer();

		for (Sample* s = buf->begin(); s != buf->end(); ++s)
			if (std::abs (*s) > max)
				max = std::abs (*s);

		levels[port] = master_volume() * max;
	}
	_ports_lock.unlock();
}


void
Backend::save_state (QDomElement& element) const
{
	QDomElement inputs = element.ownerDocument().createElement ("inputs");
	QDomElement outputs = element.ownerDocument().createElement ("outputs");

	_inputs_list->save_state (inputs);
	_outputs_list->save_state (outputs);

	element.appendChild (inputs);
	element.appendChild (outputs);
}


void
Backend::load_state (QDomElement const& element)
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
Backend::connect()
{
	try {
		_transport->connect (_client_name.toStdString());
		graph_updated();
		enable();
		QApplication::postEvent (this, new StateChange (true, false));
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Audio backend", QString ("Can't connect to audio backend: ") + e.what());
	}
	update_widgets();
}


void
Backend::disconnect()
{
	disable();
	_transport->deactivate();
	_transport->disconnect();
	QApplication::postEvent (this, new StateChange (false, false));
	update_widgets();
}


void
Backend::notify_disconnected()
{
	QApplication::postEvent (this, new StateChange (false, true));
}


void
Backend::update_widgets()
{
	_disconnect_button->setEnabled (connected());
	_reconnect_button->setEnabled (!connected());

	QTreeWidgetItem* sel = _inputs_list->selected_item();
	_destroy_input_button->setEnabled (sel != 0);

	sel = _outputs_list->selected_item();
	_destroy_output_button->setEnabled (sel != 0);
}


void
Backend::create_input()
{
	InputDialog* dialog = new InputDialog (this, this);
	if (dialog->exec() == InputDialog::Accepted)
	{
		InputItem* item = new InputItem (_inputs_list, dialog->name());
		dialog->apply (item);
		_inputs_list->setCurrentItem (item);
	}
}


void
Backend::create_input (QString const& name)
{
	InputItem* item = new InputItem (_inputs_list, name);
	_inputs_list->setCurrentItem (item);
}


void
Backend::create_output()
{
	OutputDialog* dialog = new OutputDialog (this, this);
	if (dialog->exec() == OutputDialog::Accepted)
	{
		OutputItem* item = new OutputItem (_outputs_list, dialog->name());
		dialog->apply (item);
		_outputs_list->setCurrentItem (item);
	}
}


void
Backend::create_output (QString const& name)
{
	OutputItem* item = new OutputItem (_outputs_list, name);
	_outputs_list->setCurrentItem (item);
}


void
Backend::context_menu_for_inputs (QPoint const& pos)
{
	QTreeWidgetItem* item = _inputs_list->itemAt (pos);
	QMenu* menu = new QMenu (this);
	QAction* a;

	if (item != 0)
	{
		if (dynamic_cast<InputItem*> (item) != 0)
		{
			menu->addAction (Resources::Icons16::rename(), "&Rename", this, SLOT (rename_selected_input()));
			menu->addSeparator();
			a = menu->addAction (Resources::Icons16::add(), "&New port", this, SLOT (create_input()));
			a->setEnabled (false);
			menu->addAction (Resources::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_input()));
		}
	}
	else
	{
		menu->addAction (Resources::Icons16::add(), "&New port", this, SLOT (create_input()));
		a = menu->addAction (Resources::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_input()));
		a->setEnabled (false);
	}

	menu->exec (QCursor::pos());
	delete menu;
}


void
Backend::context_menu_for_outputs (QPoint const& pos)
{
	QTreeWidgetItem* item = _outputs_list->itemAt (pos);
	QMenu* menu = new QMenu (this);
	QAction* a;

	if (item != 0)
	{
		if (dynamic_cast<OutputItem*> (item) != 0)
		{
			menu->addAction (Resources::Icons16::rename(), "&Rename", this, SLOT (rename_selected_output()));
			menu->addSeparator();
			a = menu->addAction (Resources::Icons16::add(), "&New port", this, SLOT (create_output()));
			a->setEnabled (false);
			menu->addAction (Resources::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_output()));
		}
	}
	else
	{
		menu->addAction (Resources::Icons16::add(), "&New port", this, SLOT (create_output()));
		a = menu->addAction (Resources::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_output()));
		a->setEnabled (false);
	}

	menu->exec (QCursor::pos());
	delete menu;
}


void
Backend::double_click_on_inputs (QTreeWidgetItem* item, int)
{
	if (item != 0)
	{
		item->treeWidget()->setCurrentItem (item);
		configure_selected_input();
	}
}


void
Backend::double_click_on_outputs (QTreeWidgetItem* item, int)
{
	if (item != 0)
	{
		item->treeWidget()->setCurrentItem (item);
		configure_selected_output();
	}
}


void
Backend::configure_selected_input()
{
	if (_inputs_list->selected_item())
	{
		PortItem* item = dynamic_cast<PortItem*> (_inputs_list->selected_item());
		if (item)
			item->configure();
	}
}


void
Backend::configure_selected_output()
{
	if (_outputs_list->selected_item())
	{
		PortItem* item = dynamic_cast<PortItem*> (_outputs_list->selected_item());
		if (item)
			item->configure();
	}
}


void
Backend::rename_selected_input()
{
	QTreeWidgetItem* sel = _inputs_list->selected_item();
	if (sel != 0)
		configure_selected_input();
}


void
Backend::rename_selected_output()
{
	QTreeWidgetItem* sel = _outputs_list->selected_item();
	if (sel != 0)
		configure_selected_output();
}


void
Backend::destroy_selected_input()
{
	if (_inputs_list->selected_item())
	{
		QTreeWidgetItem* item = _inputs_list->selected_item();
		_inputs_list->invisibleRootItem()->takeChild (_inputs_list->invisibleRootItem()->indexOfChild (item));
		delete item;
	}
}


void
Backend::destroy_selected_output()
{
	if (_outputs_list->selected_item())
	{
		QTreeWidgetItem* item = _outputs_list->selected_item();
		_outputs_list->invisibleRootItem()->takeChild (_inputs_list->invisibleRootItem()->indexOfChild (item));
		delete item;
	}
}


void
Backend::customEvent (QEvent* event)
{
	StateChange* state_change = dynamic_cast<StateChange*> (event);
	if (state_change)
	{
		if (state_change->from_backend)
		{
			update_widgets();
			on_state_change (state_change->online);
			// Show message to user:
			QMessageBox::warning (this, "Audio backend", "Audio transport disconnected.\nUse «Reconnect» button on Audio backend tab (or press C-j) to reconnect.");
		}
		else
			on_state_change (state_change->online);
	}
}


void
Backend::graph_updated()
{
	Unit::graph_updated();
	// Keep smoothing time independent from sample rate:
	_master_volume_smoother.set_samples (0.01f * graph()->sample_rate());
	_master_volume_smoother_buffer.resize (graph()->buffer_size());
}


void
Backend::dummy_round()
{
	const int DummyPeriodTime = 33; // ms
	const int DummySampleRate = 48000;
	const int DummyBufferSize = DummySampleRate / (1000.0 / DummyPeriodTime);
	graph()->lock();
	graph()->set_sample_rate (DummySampleRate);
	graph()->set_buffer_size (DummyBufferSize);
	graph()->unlock();
	usleep (DummyPeriodTime * 1000); // Sleep for 33ms
}

} // namespace AudioBackendImpl

} // namespace Haruhi

