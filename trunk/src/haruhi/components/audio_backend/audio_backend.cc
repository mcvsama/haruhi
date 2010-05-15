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

// System:
#include <signal.h>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtGui/QToolTip>

// Libs:
#include <jack/jack.h>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/session.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "audio_backend.h"


namespace Haruhi {

namespace Private = AudioBackendPrivate;


AudioBackend::AudioBackend (Session* session, QString const& client_name, int id, QWidget* parent):
	Unit (0, session, "urn://haruhi.mulabs.org/backend/jack-audio-backend/1", "• Audio", id, parent),
	_client_name (client_name),
	_jack (0),
	_sample_rate (0),
	_buffer_size (0),
	_panic_pressed (false),
	_dummy_thread (this)
{
	// Block SIGPIPE to avoid terminating program due to failure on JACK read.
	ignore_sigpipe();

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
	_dummy_thread.start();
	_dummy_timer = new QTimer (this);
	QObject::connect (_dummy_timer, SIGNAL (timeout()), this, SLOT (dummy_round()));
	_dummy_timer->start (33);

	connect();
}


AudioBackend::~AudioBackend()
{
	_dummy_timer->stop();
	_dummy_thread.quit();
	// Wait for dummy_thread to be sure it's not executing
	// processing rounds:
	_dummy_thread.wait();

	disconnect();

	// Deallocate all ports:
	_inputs_list->clear();
	_outputs_list->clear();

	delete _master_volume_port;
	delete _panic_port;

	if (_jack)
	{
		::jack_client_close (_jack);
		_jack = 0;
	}

	unregister_unit();
}


void
AudioBackend::enable()
{
	Unit::enable();
	if (connected())
		::jack_activate (_jack);
}


void
AudioBackend::disable()
{
	if (connected())
		::jack_deactivate (_jack);
	Unit::disable();
}


std::size_t
AudioBackend::sample_rate()
{
	return _sample_rate;
}


std::size_t
AudioBackend::buffer_size()
{
	return _buffer_size;
}


void
AudioBackend::process()
{
	if (graph()->dummy())
		return;

	DialControl* master_volume = session()->meter_panel()->master_volume();

	// Adjust Master Volume control:
	{
		Core::EventBuffer* buffer = _master_volume_port->event_buffer();
		Core::EventBuffer::EventsMultiset const& events = buffer->events();
		for (Core::EventBuffer::EventsMultiset::const_iterator e = events.begin(); e != events.end(); ++e)
		{
			if ((*e)->event_type() == Core::Event::ControllerEventType)
			{
				Core::ControllerEvent const* controller_event = static_cast<Core::ControllerEvent const*> (e->get());
				// FIXME probably not secure: use QApplication::postEvent()...
				// FIXME or rather use QApplication::postEvent?
				master_volume->setValue (renormalize (controller_event->value(), 0.0, 1.0, Session::MeterPanel::MinVolume, Session::MeterPanel::MaxVolume));
			}
		}
	}

	// Check for Panic:
	{
		Core::EventBuffer* buffer = _panic_port->event_buffer();
		Core::EventBuffer::EventsMultiset const& events = buffer->events();
		for (Core::EventBuffer::EventsMultiset::const_iterator e = events.begin(); e != events.end(); ++e)
		{
			if ((*e)->event_type() == Core::Event::ControllerEventType)
			{
				Core::ControllerEvent const* controller_event = static_cast<Core::ControllerEvent const*> (e->get());
				if (controller_event->value() >= 0.5 && _panic_pressed == false)
				{
					_panic_pressed = true;
					graph()->panic();
				}
				else if (controller_event->value() < 0.5)
					_panic_pressed = false;
			}
		}
	}

	for (InputsSet::iterator p = _inputs.begin();  p != _inputs.end();  ++p)
		(*p)->transfer();

	sync_inputs();

	// Use Master Volume control to adjust volume of outputs:
	// FIXME probably non-secure, use exported int value (using Atomic).
	Core::Sample v = std::pow (master_volume->value() / static_cast<float> (Session::MeterPanel::ZeroVolume), M_E);
	for (OutputsSet::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
		(*p)->attenuate (v);

	for (OutputsSet::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
		(*p)->transfer();

	// Level meter:
	{
		std::vector<Private::OutputItem*> ovec;
		std::copy (_outputs.begin(), _outputs.end(), std::back_inserter (ovec));
		std::sort (ovec.begin(), ovec.end(), Private::PortItem::CompareByPortName());
		for (unsigned int i = 0; i < std::min (ovec.size(), static_cast<std::vector<Private::OutputItem*>::size_type> (2u)); ++i)
		{
			Core::AudioBuffer* abuf = ovec[i]->port()->audio_buffer();
			session()->meter_panel()->level_meters_group()->meter (i)->process (abuf->begin(), abuf->end());
		}
	}
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
			else if (e.tagName() == "outputs")
				_outputs_list->load_state (e);
		}
	}
	if (e)
		enable();
}


void
AudioBackend::connect()
{
	if (connected())
		disconnect();

	void* vthis = static_cast<void*> (this);

	try {
		_dummy_timer->stop();

		if (!(_jack = ::jack_client_open (_client_name.toUtf8(), (jack_options_t)JackNoStartServer, 0)))
			throw AudioBackendException ("could not connect to JACK server - is it running?", __func__);

		if (::jack_set_process_callback (_jack, static_cb_process, vthis))
			throw AudioBackendException ("could not setup process callback", __func__);

		if (::jack_set_sample_rate_callback (_jack, static_cb_sample_rate_change, vthis))
			throw AudioBackendException ("could not setup sample rate change callback", __func__);

		if (::jack_set_buffer_size_callback (_jack, static_cb_buffer_size_change, vthis))
			throw AudioBackendException ("could not setup buffer size change callback", __func__);

		::jack_on_shutdown (_jack, static_cb_shutdown, vthis);

		_sample_rate = ::jack_get_sample_rate (_jack);
		_buffer_size = ::jack_get_buffer_size (_jack);

		update_graph();

		// Reconnect all ports:
		for (InputsSet::iterator p = _inputs.begin(); p != _inputs.end(); ++p)
			(*p)->initialize();
		for (OutputsSet::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
			(*p)->initialize();

		enable();
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Audio backend", QString ("Can't start audio backend: ") + e.what());
		disconnect();
	}
	update_widgets();
}


void
AudioBackend::disconnect()
{
	if (_jack)
	{
		disable();
		invalidate_ports();
		// Close JACK client:
		::jack_client_close (_jack);
		_jack = 0;
		update_widgets();
	}
	_dummy_timer->start();
}


bool
AudioBackend::connected() const
{
	return _jack != 0;
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
	_dummy_thread.execute();
}


void
AudioBackend::customEvent (QEvent* event)
{
	OfflineNotification* offline_notification = dynamic_cast<OfflineNotification*> (event);
	if (offline_notification)
		jack_disconnected();
}


void
AudioBackend::graph_updated()
{
	Unit::graph_updated();
	// Update smoothers for all OutputItems:
	for (OutputsSet::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
		(*p)->graph_updated();
}


void
AudioBackend::ignore_sigpipe()
{
	sigset_t set;
	sigemptyset (&set);
	sigaddset (&set, SIGPIPE);
	sigprocmask (SIG_BLOCK, &set, 0);
}


void
AudioBackend::update_graph()
{
	// Update graph parameters:
	graph()->lock();
	graph()->set_buffer_size (_buffer_size);
	graph()->set_sample_rate (_sample_rate);
	graph()->unlock();
}


void
AudioBackend::invalidate_ports()
{
	// Invalidate all ports:
	for (InputsSet::iterator p = _inputs.begin(); p != _inputs.end(); ++p)
		(*p)->invalidate();
	for (OutputsSet::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
		(*p)->invalidate();
}


void
AudioBackend::offline_invalidate_ports()
{
	// Invalidate all ports:
	for (InputsSet::iterator p = _inputs.begin(); p != _inputs.end(); ++p)
		(*p)->offline_invalidate();
	for (OutputsSet::iterator p = _outputs.begin(); p != _outputs.end(); ++p)
		(*p)->offline_invalidate();
}


void
AudioBackend::jack_disconnected()
{
	_jack = 0;
	disable();
	offline_invalidate_ports();
	_dummy_timer->start();
	// Show message to user:
	QMessageBox::warning (this, "Audio backend", "JACK disconnected. :[\nUse 'Reconnect' button on Audio backend tab to reconnect to JACK.");
}


int
AudioBackend::cb_process (jack_nframes_t)
{
	if (enabled())
	{
		session()->graph()->enter_processing_round();
		if (enabled())
			sync();
		session()->graph()->leave_processing_round();
	}
	return 0;
}


int
AudioBackend::cb_sample_rate_change (jack_nframes_t srate)
{
	_sample_rate = srate;
	return 0;
}


int
AudioBackend::cb_buffer_size_change (jack_nframes_t nframes)
{
	_buffer_size = nframes;
	return 0;
}


/**
 * Warning: this function is ASYNC, and should use async-safe
 * functions only.
 */
void
AudioBackend::cb_shutdown()
{
	QApplication::postEvent (this, new OfflineNotification());
}


int
AudioBackend::static_cb_process (jack_nframes_t nframes, void* klass)
{
	// Call appropiate backend instance member:
	return reinterpret_cast<AudioBackend*> (klass)->cb_process (nframes);
}


int
AudioBackend::static_cb_sample_rate_change (jack_nframes_t srate, void* klass)
{
	// Call appropiate backend instance member:
	return reinterpret_cast<AudioBackend*> (klass)->cb_sample_rate_change (srate);
}


int
AudioBackend::static_cb_buffer_size_change (jack_nframes_t nframes, void* klass)
{
	// Call appropiate backend instance member:
	return reinterpret_cast<AudioBackend*> (klass)->cb_buffer_size_change (nframes);
}


void
AudioBackend::static_cb_shutdown (void* klass)
{
	// Call appropiate backend instance member:
	reinterpret_cast<AudioBackend*> (klass)->cb_shutdown();
}

} // namespace Haruhi

