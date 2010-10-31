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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__EVENT_BACKEND_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__EVENT_BACKEND_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdint.h>
#include <set>

// Qt:
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QStackedWidget>
#include <QtXml/QDomNode>
#include <Qt3Support/Q3PopupMenu>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/config.h>
#include <haruhi/core/event.h>
#include <haruhi/core/event_port.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/backend.h>
#include <haruhi/unit.h>

// Local:
#include "event_transport.h"
#include "ports_list_view.h"
#include "port_item.h"
#include "device_with_port_item.h"
#include "controller_with_port_item.h"
#include "device_dialog.h"
#include "controller_dialog.h"


namespace Haruhi {

class EventBackend:
	public Unit,
	public SaveableState,
	public Backend
{
	Q_OBJECT

	friend class EventBackendPrivate::DeviceWithPortItem;
	friend class EventBackendPrivate::ControllerWithPortItem;

  public:
	/**
	 * Ports to listen for in start_learning()/stop_learning().
	 * As transport type use EventTypes.
	 */
	enum {
		Keyboard			= 1 << 0,
		Controller			= 1 << 1,
		Pitchbend			= 1 << 2,
		ChannelPressure		= 1 << 3,
		KeyPressure			= 1 << 4,
	};

	typedef uint32_t EventTypes;

	/**
	 * Objects deriving this class can use EventBackend
	 * to automatically learn which event port to use
	 * for eg. given knob.
	 * See start_learning() and stop_learning() methods.
	 */
	class Learnable
	{
	  public:
		/**
		 * Will be called when port is learned.
		 * \entry	Synth thread.
		 */
		virtual void
		learned_port (EventTypes event_types, Core::EventPort* event_port) = 0;
	};

  private:
	typedef std::set<std::pair<Learnable*, EventTypes> > Learnables;
	typedef std::map<EventTransport::Port*, EventBackendPrivate::DeviceWithPortItem*> InputsMap;
	typedef std::map<int, Config::EventHardwareTemplate> Templates;

  public:
	EventBackend (Session*, QString const& client_name, int id, QWidget* parent);

	~EventBackend();

	EventTransport*
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
	void
	connect();

	void
	disconnect();

	bool
	connected() const;

	void
	configure_item (EventBackendPrivate::DeviceWithPortItem* item);

	void
	configure_item (EventBackendPrivate::ControllerWithPortItem* item);

	/**
	 * Starts listening for specified event types.
	 * When specified event arrive on any input port Learnable
	 * is notified about it.
	 */
	void
	start_learning (Learnable*, EventTypes);

	/**
	 * Stops learning started with start_learning(),
	 * eg. when user changes his mind.
	 */
	void
	stop_learning (Learnable*, EventTypes);

	void
	update_widgets();

	void
	selection_changed();

	void
	create_device();

	void
	create_controller();

	void
	context_menu_for_inputs (QPoint const&);

	void
	configure_selected_input();

	void
	learn_from_midi();

	void
	destroy_selected_input();

	void
	save_selected_input();

  private:
	/**
	 * Creates popup menu with templates for insertion
	 * and stores it in _templates_menu.
	 */
	Q3PopupMenu*
	create_templates_menu();

	void
	handle_event_for_learnables (EventTransport::MidiEvent const&, Core::EventPort*);

  private slots:
	/**
	 * Inserts new input hardware template into list.
	 * Uses menu_item_id from _templates map.
	 */
	void
	insert_template (int menu_item_id);

  private:
	QString									_client_name;
	EventTransport*							_transport;
	InputsMap								_inputs;
	Learnables								_learnables;

	// Widgets:
	QPushButton*							_create_device_button;
	QPushButton*							_create_controller_button;
	QPushButton*							_destroy_input_button;
	QStackedWidget*							_stack;
	EventBackendPrivate::DeviceDialog*		_device_dialog;
	EventBackendPrivate::ControllerDialog*	_controller_dialog;
	EventBackendPrivate::PortsListView*		_tree;

	// External (hardware) port templates menu and helper storage:
	Templates								_templates;
	Q3PopupMenu*							_templates_menu;
};


/**
 * EventBackendException
 */
class EventBackendException: public Exception
{
  public:
	explicit EventBackendException (const char* what, const char* details):
		Exception (what, details)
	{ }
};


/**
 * EventBackendPortException
 */
class EventBackendPortException: public EventBackendException
{
  public:
	explicit EventBackendPortException (const char* what, const char* details):
		EventBackendException (what, details)
	{ }
};

} // namespace Haruhi

#endif

