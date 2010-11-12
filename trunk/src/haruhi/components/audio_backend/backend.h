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
 *
 * NOTES
 *
 * Never call JACK functions when session's graph lock is acquired. This may lead to deadlock
 * when JACK will wait for end of its graph processing, and process() function will wait
 * on session's graph lock.
 *
 * As there is assumption that in processing round graph cannot be modified you should only
 * acquire graph lock when changing core objects (ports, unit attributes, connecting ports, etc.).
 */

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__BACKEND_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__BACKEND_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>
#include <utility>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QWhatsThis>
#include <QtGui/QDialog>
#include <QtGui/QTreeWidgetItem>
#include <QtXml/QDomNode>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/haruhi.h>
#include <haruhi/backend.h>
#include <haruhi/core/unit.h>
#include <haruhi/core/audio_buffer.h>
#include <haruhi/core/event_port.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/mutex.h>

// Local:
#include "ports_list_view.h"
#include "port_item.h"
#include "input_item.h"
#include "output_item.h"
#include "port_dialog.h"
#include "input_dialog.h"
#include "output_dialog.h"
#include "transport.h"


namespace Haruhi {

namespace AudioBackend {

class Backend:
	public QWidget,
	public Unit,
	public SaveableState,
	public ::Haruhi::Backend
{
	Q_OBJECT

	class OfflineNotification: public QEvent
	{
	  public:
		OfflineNotification():
			QEvent (QEvent::User)
		{ }
	};

	friend class PortItem;
	friend class InputItem;
	friend class OutputItem;

  private:
	typedef std::map<Transport::Port*, InputItem*>	InputsMap;
	typedef std::map<Transport::Port*, OutputItem*>	OutputsMap;

  public:
	Backend (Session* session, QString const& client_name, int id, QWidget* parent);

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

	EventPort*
	master_volume_port() const { return _master_volume_port; }

	EventPort*
	panic_port() const { return _panic_port; }

	/**
	 * Starts processing.
	 */
	void
	enable();

	/**
	 * Stops processing. Backend must not enter processing round after this call ends.
	 * Otherwise system behavior is undefined.
	 */
	void
	disable();

	/**
	 * Transports data between graph's ports and Transport's ports.
	 */
	void
	transfer();

	/*
	 * Haruhi::Unit methods:
	 */

	void
	process();

	/*
	 * SaveableState methods:
	 */

	/**
	 * \entry	Qt thread only.
	 */
	void
	save_state (QDomElement&) const;

	/**
	 * \entry	Qt thread only.
	 */
	void
	load_state (QDomElement const&);

  public slots:
	/**
	 * Connects backend to underlying transport to
	 * allow operation.
	 */
	void
	connect();

	/**
	 * Disconnects backend from transport.
	 */
	void
	disconnect();

	/**
	 * Returns true if backend is connected to transport.
	 */
	bool
	connected() const;

	/**
	 * \threadsafe
	 */
	void
	notify_disconnected();

	void
	update_widgets();

	void
	create_input();

	void
	create_input (QString const& name);

	void
	create_output();

	void
	create_output (QString const& name);

	void
	context_menu_for_inputs (QPoint const&);

	void
	context_menu_for_outputs (QPoint const&);

	void
	double_click_on_inputs (QTreeWidgetItem*, int);

	void
	double_click_on_outputs (QTreeWidgetItem*, int);

	void
	configure_selected_input();

	void
	configure_selected_output();

	void
	rename_selected_input();

	void
	rename_selected_output();

	void
	destroy_selected_input();

	void
	destroy_selected_output();

  private slots:
	void
	dummy_start();

	void
	dummy_stop();

	void
	dummy_round();

	void
	update_level_meter();

  protected:
	void
	customEvent (QEvent* event);

	void
	graph_updated();

  private:
	QString				_client_name;
	Transport*			_transport;
	RecursiveMutex		_ports_lock;

	// Views:
	QPushButton*		_disconnect_button;
	QPushButton*		_reconnect_button;
	QPushButton*		_create_input_button;
	QPushButton*		_destroy_input_button;
	QPushButton*		_create_output_button;
	QPushButton*		_destroy_output_button;

	// Ports lists:
	PortsListView*		_inputs_list;
	PortsListView*		_outputs_list;

	// Master volume control port:
	EventPort*			_master_volume_port;
	EventPort*			_panic_port;

	// Used for graph ticks when audio backend is not running:
	QTimer*				_dummy_timer;

	// Ports sets:
	InputsMap			_inputs;
	OutputsMap			_outputs;
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
 * AudioBackendPortException
 */
class PortException: public Exception
{
  public:
	explicit PortException (const char* what, const char* details):
		Exception (what, details)
	{ }
};

} // namespace AudioBackend

} // namespace Haruhi

#endif

