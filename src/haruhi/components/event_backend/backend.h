/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__BACKEND_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__BACKEND_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdint.h>
#include <set>

// Qt:
#include <QtCore/QSignalMapper>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QStackedWidget>
#include <QtGui/QMenu>
#include <QtXml/QDomNode>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/components/devices_manager/device.h>
#include <haruhi/components/devices_manager/device_dialog.h>
#include <haruhi/components/devices_manager/controller_dialog.h>
#include <haruhi/components/devices_manager/model.h>
#include <haruhi/graph/event.h>
#include <haruhi/graph/event_backend.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/lib/midi.h>
#include <haruhi/settings/settings.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/exception.h>
#include <haruhi/utility/signal.h>

// Local:
#include "tree.h"
#include "port_item.h"
#include "device_with_port_item.h"
#include "controller_with_port_item.h"
#include "transport.h"


namespace Haruhi {

namespace EventBackendImpl {

class DeviceWithPortDialog;
class ControllerWithPortDialog;

class Backend:
	public QWidget,
	public SaveableState,
	public EventBackend,
	public Signal::Receiver
{
	Q_OBJECT

	class StateChange: public QEvent
	{
	  public:
		StateChange (bool online):
			QEvent (QEvent::User),
			online (online)
		{ }

		// True if just connected, false if disconnected:
		bool online;
	};

	friend class DeviceWithPortItem;
	friend class ControllerWithPortItem;

  private:
	typedef std::map<Transport::Port*, DeviceWithPortItem*>	InputsMap;
	typedef std::map<int, DevicesManager::Device>			Templates;

  public:
	Backend (QString const& client_name, QWidget* parent);

	~Backend();

	/**
	 * Returns true, if backend is connected to transport.
	 */
	bool
	connected() const;

	/**
	 * Inserts given Device into list.
	 */
	void
	add_device (DevicesManager::Device const& device);

	/**
	 * Returns event transport used by backend.
	 */
	Transport*
	transport() const;

	/**
	 * Should be called whenever DevicesManager's model change.
	 * Updates list of template devices.
	 */
	void
	devices_manager_updated();

	/*
	 * Unit API
	 */

	void
	registered();

	void
	unregistered();

	void
	process();

	/*
	 * SaveableState API
	 * EventBackend saves/restores lists of devices and controllers.
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

	/**
	 * Connects to backend to transport to allow operation.
	 * Done automatically after unit is registered.
	 */
	void
	connect();

	/**
	 * Disconnects backend from transport.
	 * Done automatically before unit is unregistered.
	 */
	void
	disconnect();

  public:
	/**
	 * Emited after user saves Device as template. Can be used to inform DevicesManager to save new device template.
	 * If nothing is connected to this signal, 'save as template' menu item will be hidden in device popup menu.
	 * \param	device Device to be saved as template.
	 * \thread	UI thread
	 */
	Signal::Emiter1<DevicesManager::Device const&> device_saved_as_template;

	/**
	 * Emited on each event from transport.
	 * \thread	Engine thread
	 */
	Signal::Emiter1<MIDI::Event const&> on_event;

  private slots:
	/**
	 * Updates widgets (enables/disables buttons, etc)
	 * depending on current UI state.
	 */
	void
	update_widgets();

	/**
	 * Emits signal device_saved_as_template()
	 * if currently selected item is device item.
	 */
	void
	save_selected_item_as_template();

	/**
	 * Locates item using given point, creates and
	 * executes menu for the item.
	 */
	void
	context_menu_for_items (QPoint const&);

	/**
	 * Creates/updates menu with templates for insertion and stores it in _templates_menu.
	 */
	void
	update_templates_menu();

	/**
	 * Handles event notification from transport.
	 * Finds learnables waiting for event and notifies
	 * them about event.
	 *
	 * This is used to learn connections between event backend ports
	 * and learning ports.
	 */
	void
	handle_event_for_learnables (MIDI::Event const& event, EventPort* port);

	/**
	 * Updates widgets and calls configure_selected_item().
	 */
	void
	selection_changed();

	/**
	 * Displays configuration dialog for given device item.
	 */
	void
	configure_item (DeviceItem* item);

	/**
	 * Displays configuration dialog for given controller item.
	 */
	void
	configure_item (ControllerItem* item);

	/**
	 * Detects currently selected item and displays
	 * configuration dialog for it.
	 */
	void
	configure_selected_item();

	/**
	 * Puts currently selected item into learning mode
	 * (used to configure filters in the item).
	 */
	void
	learn_from_midi();

	/**
	 * Inserts new device template into list.
	 * Uses _templates[id].
	 */
	void
	add_template (int template_id);

  protected:
	void
	customEvent (QEvent* event);

  private:
	QString						_client_name;
	Transport*					_transport;
	InputsMap					_inputs;
	QSignalMapper*				_insert_template_signal_mapper;
	DevicesManager::Model		_model;

	// Widgets:
	QStackedWidget*				_stack;
	Tree*						_tree;
	QPushButton*				_create_device_button;
	QPushButton*				_create_controller_button;
	QPushButton*				_destroy_input_button;
	QPushButton*				_insert_template_button;
	DeviceWithPortDialog*		_device_dialog;
	ControllerWithPortDialog*	_controller_dialog;

	// Device templates menu and helper storage:
	Templates					_templates;
	QMenu*						_templates_menu;
};


/**
 * Exception
 */
class Exception: public ::Exception
{
  public:
	explicit
	Exception (const char* what, const char* details):
		::Exception (what, details)
	{ }
};


/**
 * PortException
 */
class PortException: public Exception
{
  public:
	explicit
	PortException (const char* what, const char* details):
		Exception (what, details)
	{ }
};


inline Transport*
Backend::transport() const
{
	return _transport;
}

} // namespace EventBackendImpl

} // namespace Haruhi

#endif

