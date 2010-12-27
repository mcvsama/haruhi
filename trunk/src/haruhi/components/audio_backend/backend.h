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
#include <haruhi/application/haruhi.h>
#include <haruhi/graph/audio_backend.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/exception.h>

// Local:
#include "tree.h"
#include "port_item.h"
#include "input_item.h"
#include "output_item.h"
#include "port_dialog.h"
#include "input_dialog.h"
#include "output_dialog.h"
#include "transport.h"


namespace Haruhi {

namespace AudioBackendImpl {

class Backend:
	public QWidget,
	public SaveableState,
	public AudioBackend
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
	Backend (QString const& client_name, int id, QWidget* parent);

	~Backend();

	Transport*
	transport() const { return _transport; }

	EventPort*
	master_volume_port() const { return _master_volume_port; }

	EventPort*
	panic_port() const { return _panic_port; }

	/**
	 * Starts processing, that is: enables ticks from audio subsystem.
	 * After this call engine should call periodically data_ready().
	 */
	void
	enable();

	/**
	 * Stops processing. Backend must not enter processing round after this call starts.
	 * Otherwise system behavior is undefined.
	 */
	void
	disable();

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
	 * AudioBackend API
	 */

	void
	data_ready();

	void
	peak_levels (LevelsMap& levels_map);

	/*
	 * SaveableState API
	 * Saves/restores list of input and output ports.
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
	 * \threadsafe
	 */
	void
	notify_disconnected();

  private slots:
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
	 * Updates widgets (enables/disables buttons depending on current
	 * UI state, etc).
	 */
	void
	update_widgets();

	/**
	 * Creates new input port with default name.
	 */
	void
	create_input();

	/**
	 * Creates new input port with given name.
	 * \param	name Name for new port.
	 */
	void
	create_input (QString const& name);

	/**
	 * Creates new output port with default name.
	 */
	void
	create_output();

	/**
	 * Creates new output port with given name.
	 * \param	name Name for new port.
	 */
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

  protected:
	void
	customEvent (QEvent* event);

	/**
	 * Unit API
	 */
	void
	graph_updated();

  private:
	void
	dummy_round();

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
	Tree*				_inputs_list;
	Tree*				_outputs_list;

	// Master volume control port:
	EventPort*			_master_volume_port;
	EventPort*			_panic_port;

	// Ports sets:
	InputsMap			_inputs;
	OutputsMap			_outputs;
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
 * AudioBackendPortException
 */
class PortException: public Exception
{
  public:
	explicit PortException (const char* what, const char* details):
		Exception (what, details)
	{ }
};

} // namespace AudioBackendImpl

} // namespace Haruhi

#endif

