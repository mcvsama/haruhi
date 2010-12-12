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
#include <haruhi/components/devices_manager/device_dialog.h>
#include <haruhi/components/devices_manager/controller_dialog.h>
#include <haruhi/graph/event.h>
#include <haruhi/graph/event_backend.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/settings/settings.h>
#include <haruhi/settings/devices_manager_settings.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/exception.h>

// Local:
#include "ports_list_view.h"
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
	public EventBackend
{
	Q_OBJECT

	friend class DeviceWithPortItem;
	friend class ControllerWithPortItem;

  private:
	typedef std::map<Transport::Port*, DeviceWithPortItem*>	InputsMap;
	typedef std::map<int, DevicesManagerSettings::Device>	Templates;

  public:
	Backend (QString const& client_name, int id, QWidget* parent);

	~Backend();

	/**
	 * Returns event transport used by backend.
	 */
	Transport*
	transport() const { return _transport; }

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

  signals:
	/**
	 * Emited after user saves Device as template.
	 * Can be used to inform DevicesManager to save new device template.
	 */
	void
	device_saved_as_template (DeviceItem* device_item);

  private slots:
	/**
	 * Updates widgets (enables/disables buttons, etc)
	 * depending on current UI state.
	 */
	void
	update_widgets();

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

	/**
	 * Returns true, if backend is connected to transport.
	 */
	bool
	connected() const;

	/**
	 * Creates new unnamed/unconfigured device
	 * and inserts it into the tree.
	 */
	void
	create_device();

	/**
	 * Creates new unnamed/unconfigured controller
	 * and inserts it into the subtree of currently selected device.
	 * If no device is selected, it does nothing.
	 */
	void
	create_controller();

	/**
	 * Destroys currently selected device or controller.
	 */
	void
	destroy_selected_item();

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
	 * Creates popup menu with templates for insertion
	 * and stores it in _templates_menu.
	 */
	void
	create_templates_menu (QMenu* menu);

	/**
	 * Handles event notification from transport.
	 * Finds learnables waiting for event and notifies
	 * them about event.
	 */
	void
	handle_event_for_learnables (Transport::MidiEvent const& event, EventPort* port);

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
	 * Puts currently selected item into learning mode.
	 */
	void
	learn_from_midi();

	/**
	 * Inserts new device template into list.
	 * Uses _templates[id].
	 */
	void
	insert_template (int template_id);

  private:
	QString						_client_name;
	Transport*					_transport;
	InputsMap					_inputs;
	QSignalMapper*				_insert_template_signal_mapper;

	// Widgets:
	QPushButton*				_create_device_button;
	QPushButton*				_create_controller_button;
	QPushButton*				_destroy_input_button;
	QStackedWidget*				_stack;
	PortsListView*				_tree;
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
	explicit Exception (const char* what, const char* details):
		::Exception (what, details)
	{ }
};


/**
 * PortException
 */
class PortException: public Exception
{
  public:
	explicit PortException (const char* what, const char* details):
		Exception (what, details)
	{ }
};

} // namespace EventBackendImpl

} // namespace Haruhi

#endif

