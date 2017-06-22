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
#include <cstdlib>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>

// Qt:
#include <QApplication>
#include <QLayout>
#include <QMenu>
#include <QLabel>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/qdom.h>
#include <haruhi/widgets/styled_background.h>

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
	_master_volume_port = std::make_unique<EventPort> (this, "Master Volume", Port::Input);
	_panic_port = std::make_unique<EventPort> (this, "Panic", Port::Input);

	_transport = std::make_unique<JackTransport> (this);

	_connect_retry_timer = std::make_unique<QTimer> (this);
	QObject::connect (_connect_retry_timer.get(), SIGNAL (timeout()), this, SLOT (connect()));

	_disconnect_button = std::make_unique<QPushButton> (Resources::Icons16::disconnect(), "Disconnect from JACK", this);
	_disconnect_button->setIconSize (Resources::Icons16::haruhi().size());
	_disconnect_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QObject::connect (_disconnect_button.get(), SIGNAL (clicked()), this, SLOT (disconnect()));

	_reconnect_button = std::make_unique<QPushButton> (Resources::Icons16::connect(), "Reconnect to JACK", this);
	_reconnect_button->setIconSize (Resources::Icons16::haruhi().size());
	_reconnect_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QObject::connect (_reconnect_button.get(), SIGNAL (clicked()), this, SLOT (connect()));

	_create_input_button = std::make_unique<QPushButton> (Resources::Icons16::add(), "New input", this);
	_create_input_button->setIconSize (Resources::Icons16::haruhi().size());
	_create_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_create_input_button->setToolTip ("Create new input port");
	_create_input_button->setFlat (true);
	QObject::connect (_create_input_button.get(), SIGNAL (clicked()), this, SLOT (create_input()));

	_destroy_input_button = std::make_unique<QPushButton> (Resources::Icons16::remove(), "Destroy", this);
	_destroy_input_button->setIconSize (Resources::Icons16::haruhi().size());
	_destroy_input_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_destroy_input_button->setToolTip ("Destroy selected port");
	_destroy_input_button->setFlat (true);
	QObject::connect (_destroy_input_button.get(), SIGNAL (clicked()), this, SLOT (destroy_selected_input()));

	_create_output_button = std::make_unique<QPushButton> (Resources::Icons16::add(), "New output", this);
	_create_output_button->setIconSize (Resources::Icons16::haruhi().size());
	_create_output_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_create_output_button->setToolTip ("Create new output port");
	_create_output_button->setFlat (true);
	QObject::connect (_create_output_button.get(), SIGNAL (clicked()), this, SLOT (create_output()));

	_destroy_output_button = std::make_unique<QPushButton> (Resources::Icons16::remove(), "Destroy", this);
	_destroy_output_button->setIconSize (Resources::Icons16::haruhi().size());
	_destroy_output_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_destroy_output_button->setToolTip ("Destroy selected port");
	_destroy_output_button->setFlat (true);
	QObject::connect (_destroy_output_button.get(), SIGNAL (clicked()), this, SLOT (destroy_selected_output()));

	_inputs_list = std::make_unique<Tree> (this, this, "Inputs");
	QObject::connect (_inputs_list.get(), SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_inputs (const QPoint&)));
	QObject::connect (_inputs_list.get(), SIGNAL (itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT (double_click_on_inputs (QTreeWidgetItem*, int)));
	QObject::connect (_inputs_list.get(), SIGNAL (itemSelectionChanged()), this, SLOT (update_widgets()));

	_outputs_list = std::make_unique<Tree> (this, this, "Outputs");
	QObject::connect (_outputs_list.get(), SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu_for_outputs (const QPoint&)));
	QObject::connect (_outputs_list.get(), SIGNAL (itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT (double_click_on_outputs (QTreeWidgetItem*, int)));
	QObject::connect (_outputs_list.get(), SIGNAL (itemSelectionChanged()), this, SLOT (update_widgets()));

	// Layouts:

	auto input_buttons_layout = new QHBoxLayout();
	input_buttons_layout->setSpacing (Config::spacing());
	input_buttons_layout->addWidget (_create_input_button.get());
	input_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	input_buttons_layout->addWidget (_destroy_input_button.get());

	auto inputs_layout = new QVBoxLayout();
	inputs_layout->setSpacing (Config::spacing());
	inputs_layout->addLayout (input_buttons_layout);
	inputs_layout->addWidget (_inputs_list.get());

	auto output_buttons_layout = new QHBoxLayout();
	output_buttons_layout->setSpacing (Config::spacing());
	output_buttons_layout->addWidget (_create_output_button.get());
	output_buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	output_buttons_layout->addWidget (_destroy_output_button.get());

	auto outputs_layout = new QVBoxLayout();
	outputs_layout->setSpacing (Config::spacing());
	outputs_layout->addLayout (output_buttons_layout);
	outputs_layout->addWidget (_outputs_list.get());

	auto jack_layout = new QHBoxLayout();
	jack_layout->setSpacing (Config::spacing());
	jack_layout->addWidget (_disconnect_button.get());
	jack_layout->addWidget (_reconnect_button.get());
	jack_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	auto layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addWidget (new StyledBackground (new QLabel ("Audio inputs"), this));
	layout->addLayout (inputs_layout);
	layout->addWidget (new StyledBackground (new QLabel ("Audio outputs"), this));
	layout->addLayout (outputs_layout);
	layout->addWidget (new StyledBackground (new QLabel ("JACK control"), this));
	layout->addLayout (jack_layout);

	update_widgets();
	Unit::enable();
}


Backend::~Backend()
{
}


bool
Backend::connected() const
{
	return _transport->connected();
}


void
Backend::start_processing()
{
	_transport->activate();
}


void
Backend::stop_processing()
{
	_transport->deactivate();
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

	_ports_lock.synchronize ([&] {
		// Use Master Volume control to adjust volume of outputs:
		_master_volume_smoother.fill (_master_volume_smoother_buffer.begin(),
									  _master_volume_smoother_buffer.end(),
									  master_volume());
		for (auto& p: _outputs)
			if (p.second->ready())
				p.second->port()->buffer()->attenuate (&_master_volume_smoother_buffer);

		// Copy data from graph to transport (output):
		_transport->lock_ports();
		for (auto& p: _outputs)
		{
			if (!p.second->ready())
				continue;
			if (p.second->port()->back_connections().empty())
				p.first->buffer()->clear();
			else
				p.first->buffer()->fill (p.second->port()->buffer());
		}
		_transport->unlock_ports();
	});

	_transport->data_ready();

	// Copy data from transport to graph (input):
	_ports_lock.synchronize ([&] {
		_transport->lock_ports();
		for (auto& p: _inputs)
			if (p.second->ready())
				p.first->buffer()->fill (p.second->port()->buffer());
		_transport->unlock_ports();
	});
}


void
Backend::peak_levels (LevelsMap& levels)
{
	levels.clear();

	Mutex::Lock lock (_ports_lock);

	for (auto& p: _outputs)
	{
		Sample max = 0.0f;
		AudioPort* port = p.second->port();
		AudioBuffer* buf = port->buffer();

		for (Sample s: *buf)
			if (std::abs (s) > max)
				max = std::abs (s);

		levels[port] = master_volume() * max;
	}
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

	for (QDomElement& e: element)
	{
		if (e.tagName() == "inputs")
			_inputs_list->load_state (e);
		else if (e.tagName() == "outputs")
			_outputs_list->load_state (e);
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
		start_processing();
		QApplication::postEvent (this, new StateChange (true, false));
		_connect_retry_timer->stop();
	}
	catch (ConnectException const& e)
	{
		retry_connect();
	}
	update_widgets();
}


void
Backend::disconnect()
{
	stop_processing();
	_transport->deactivate();
	_transport->disconnect();
	QApplication::postEvent (this, new StateChange (false, false));
	update_widgets();
}


void
Backend::notify_disconnected()
{
	QApplication::postEvent (this, new StateChange (false, true));
	retry_connect();
}


void
Backend::update_widgets()
{
	_disconnect_button->setEnabled (connected());
	_reconnect_button->setEnabled (!connected());

	auto sel = _inputs_list->selected_item();
	_destroy_input_button->setEnabled (sel != 0);

	sel = _outputs_list->selected_item();
	_destroy_output_button->setEnabled (sel != 0);
}


void
Backend::create_input()
{
	auto dialog = new InputDialog (this, this);
	if (dialog->exec() == InputDialog::Accepted)
	{
		auto item = new InputItem (_inputs_list.get(), dialog->name());
		dialog->apply (item);
		_inputs_list->setCurrentItem (item);
	}
}


void
Backend::create_input (QString const& name)
{
	auto item = new InputItem (_inputs_list.get(), name);
	_inputs_list->setCurrentItem (item);
}


void
Backend::create_output()
{
	auto dialog = new OutputDialog (this, this);
	if (dialog->exec() == OutputDialog::Accepted)
	{
		auto item = new OutputItem (_outputs_list.get(), dialog->name());
		dialog->apply (item);
		_outputs_list->setCurrentItem (item);
	}
}


void
Backend::create_output (QString const& name)
{
	auto item = new OutputItem (_outputs_list.get(), name);
	_outputs_list->setCurrentItem (item);
}


void
Backend::context_menu_for_inputs (QPoint const& pos)
{
	auto item = _inputs_list->itemAt (pos);
	auto menu = std::make_unique<QMenu> (this);

	if (item != 0)
	{
		if (dynamic_cast<InputItem*> (item) != 0)
		{
			menu->addAction (Resources::Icons16::rename(), "&Rename", this, SLOT (rename_selected_input()));
			menu->addSeparator();
			auto a = menu->addAction (Resources::Icons16::add(), "&New port", this, SLOT (create_input()));
			a->setEnabled (false);
			menu->addAction (Resources::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_input()));
		}
	}
	else
	{
		menu->addAction (Resources::Icons16::add(), "&New port", this, SLOT (create_input()));
		auto a = menu->addAction (Resources::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_input()));
		a->setEnabled (false);
	}

	menu->exec (QCursor::pos());
}


void
Backend::context_menu_for_outputs (QPoint const& pos)
{
	auto item = _outputs_list->itemAt (pos);
	auto menu = std::make_unique<QMenu> (this);

	if (item != 0)
	{
		if (dynamic_cast<OutputItem*> (item) != 0)
		{
			menu->addAction (Resources::Icons16::rename(), "&Rename", this, SLOT (rename_selected_output()));
			menu->addSeparator();
			auto a = menu->addAction (Resources::Icons16::add(), "&New port", this, SLOT (create_output()));
			a->setEnabled (false);
			menu->addAction (Resources::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_output()));
		}
	}
	else
	{
		menu->addAction (Resources::Icons16::add(), "&New port", this, SLOT (create_output()));
		auto a = menu->addAction (Resources::Icons16::remove(), "&Destroy port", this, SLOT (destroy_selected_output()));
		a->setEnabled (false);
	}

	menu->exec (QCursor::pos());
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
		auto item = dynamic_cast<PortItem*> (_inputs_list->selected_item());
		if (item)
			item->configure();
	}
}


void
Backend::configure_selected_output()
{
	if (_outputs_list->selected_item())
	{
		auto item = dynamic_cast<PortItem*> (_outputs_list->selected_item());
		if (item)
			item->configure();
	}
}


void
Backend::rename_selected_input()
{
	auto sel = _inputs_list->selected_item();
	if (sel != 0)
		configure_selected_input();
}


void
Backend::rename_selected_output()
{
	auto sel = _outputs_list->selected_item();
	if (sel != 0)
		configure_selected_output();
}


void
Backend::destroy_selected_input()
{
	if (_inputs_list->selected_item())
	{
		auto item = _inputs_list->selected_item();
		_inputs_list->invisibleRootItem()->removeChild (item);
		delete item;
	}
}


void
Backend::destroy_selected_output()
{
	if (_outputs_list->selected_item())
	{
		auto item = _outputs_list->selected_item();
		_outputs_list->invisibleRootItem()->removeChild (item);
		delete item;
	}
}


void
Backend::customEvent (QEvent* event)
{
	auto state_change = dynamic_cast<StateChange*> (event);
	if (state_change)
	{
		if (state_change->from_backend)
		{
			update_widgets();
			on_state_change (state_change->online);
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
	_master_volume_smoother.set_samples (10_ms * graph()->sample_rate());
	_master_volume_smoother_buffer.resize (graph()->buffer_size());
}


void
Backend::dummy_round()
{
	const Time dummy_period_time = 33_ms;
	const Frequency dummy_sample_rate = 48_kHz;
	const std::size_t dummy_buffer_size = dummy_sample_rate * dummy_period_time;
	graph()->synchronize ([&] {
		graph()->set_sample_rate (dummy_sample_rate);
		graph()->set_buffer_size (dummy_buffer_size);
	});
	usleep (dummy_period_time.us());
}


void
Backend::retry_connect()
{
	_connect_retry_timer->start (1000);
}

} // namespace AudioBackendImpl

} // namespace Haruhi

