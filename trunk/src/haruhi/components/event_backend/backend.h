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
#include <haruhi/backend.h>
#include <haruhi/components/devices_manager/device_dialog.h>
#include <haruhi/components/devices_manager/controller_dialog.h>
#include <haruhi/haruhi.h>
#include <haruhi/config.h>
#include <haruhi/graph/event.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/graph/unit.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "teacher.h"
#include "ports_list_view.h"
#include "port_item.h"
#include "device_with_port_item.h"
#include "controller_with_port_item.h"
#include "transport.h"


namespace Haruhi {

namespace EventBackend {

class DeviceWithPortDialog;
class ControllerWithPortDialog;

class Backend:
	public QWidget,
	public Unit,
	public Teacher,
	public SaveableState,
	public ::Haruhi::Backend
{
	Q_OBJECT

	friend class DeviceWithPortItem;
	friend class ControllerWithPortItem;

  private:
	typedef std::map<Transport::Port*, DeviceWithPortItem*> InputsMap;
	typedef std::map<int, Config::EventHardwareTemplate> Templates;

  public:
	Backend (Session*, QString const& client_name, int id, QWidget* parent);

	~Backend();

	/**
	 * Unit API
	 */
	void
	registered();

	/**
	 * Unit API
	 */
	void
	unregistered();

	Transport*
	transport() const { return _transport; }

	/*
	 * Haruhi::Unit methods:
	 */

	void
	process();

	/*
	 * Saveable methods:
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  public slots:
	/**
	 * Connects to backend to transport to allow operation.
	 */
	void
	connect();

	/**
	 * Disconnects backend from transport.
	 */
	void
	disconnect();

	/**
	 * Returns true, if backend is connected to transport.
	 */
	bool
	connected() const;

	void
	update_widgets();

	void
	selection_changed();

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

	void
	configure_item (DeviceItem* item);

	void
	configure_item (ControllerItem* item);

	void
	configure_selected_item();

	void
	learn_from_midi();

	void
	destroy_selected_item();

	void
	save_selected_item();

	void
	context_menu_for_items (QPoint const&);

  private:
	/**
	 * Creates popup menu with templates for insertion
	 * and stores it in _templates_menu.
	 */
	void
	create_templates_menu (QMenu* menu);

  private slots:
	/**
	 * Inserts new input hardware template into list.
	 * Uses menu_item_id from _templates map.
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

	// External (hardware) port templates menu and helper storage:
	Templates					_templates;
	QMenu*						_templates_menu;
};


/**
 * Exception
 */
class Exception: public ::Haruhi::Exception
{
  public:
	explicit Exception (const char* what, const char* details):
		::Haruhi::Exception (what, details)
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

} // namespace EventBackend

} // namespace Haruhi

#endif

