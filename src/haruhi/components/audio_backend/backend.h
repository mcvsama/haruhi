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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__BACKEND_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__BACKEND_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>
#include <utility>

// Qt:
#include <QTimer>
#include <QPushButton>
#include <QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/dsp/one_pole_smoother.h>
#include <haruhi/graph/audio_backend.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/exception.h>

// Local:
#include "exception.h"
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

/**
 * Don't ever disable this unit, it never wants to be disabled
 * from processing rounds.
 */
class Backend:
	public QWidget,
	public SaveableState,
	public AudioBackend
{
	Q_OBJECT

	class StateChange: public QEvent
	{
	  public:
		StateChange (bool online, bool from_backend);

		// True if just connected, false if disconnected:
		bool online;
		// Whether notification was caused by transport:
		bool from_backend;
	};

	friend class PortItem;
	friend class InputItem;
	friend class OutputItem;

  private:
	typedef std::map<Transport::Port*, InputItem*>	InputsMap;
	typedef std::map<Transport::Port*, OutputItem*>	OutputsMap;

  public:
	Backend (QString const& client_name, QWidget* parent);

	~Backend();

	Transport*
	transport() const;

	/**
	 * Starts processing, that is: enables ticks from audio subsystem.
	 * After this call engine should call periodically data_ready().
	 */
	void
	start_processing();

	/**
	 * Stops processing. Backend must not enter processing round after this call starts.
	 * Otherwise system behavior is undefined.
	 */
	void
	stop_processing();

	/**
	 * Returns true if backend is connected to the transport.
	 */
	bool
	connected() const override;

	/*
	 * Unit API
	 */

	void
	registered() override;

	void
	unregistered() override;

	void
	process() override;

	/*
	 * AudioBackend API
	 */

	void
	data_ready() override;

	void
	peak_levels (LevelsMap& levels_map) override;

	EventPort*
	master_volume_port() const override;

	EventPort*
	panic_port() const override;

	/*
	 * SaveableState API
	 * Saves/restores list of input and output ports.
	 */

	/**
	 * \entry	Qt thread only.
	 */
	void
	save_state (QDomElement&) const override;

	/**
	 * \entry	Qt thread only.
	 */
	void
	load_state (QDomElement const&) override;

  public slots:
	/**
	 * Connects backend to underlying transport to
	 * allow operation.
	 */
	void
	connect() override;

	/**
	 * Disconnects backend from transport.
	 */
	void
	disconnect() override;

	/**
	 * \threadsafe
	 */
	void
	notify_disconnected();

  private slots:
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
	create_input() override;

	/**
	 * Creates new input port with given name.
	 * \param	name Name for new port.
	 */
	void
	create_input (QString const& name) override;

	/**
	 * Creates new output port with default name.
	 */
	void
	create_output() override;

	/**
	 * Creates new output port with given name.
	 * \param	name Name for new port.
	 */
	void
	create_output (QString const& name) override;

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
	customEvent (QEvent* event) override;

	/**
	 * Unit API
	 */
	void
	graph_updated() override;

  private:
	void
	dummy_round();

	void
	retry_connect();

  private:
	QString					_client_name;
	Unique<Transport>		_transport;
	// Lock for _inputs and _outputs:
	RecursiveMutex			_ports_lock;
	Unique<QTimer>			_connect_retry_timer;

	// Widgets:
	Unique<QPushButton>		_disconnect_button;
	Unique<QPushButton>		_reconnect_button;
	Unique<QPushButton>		_create_input_button;
	Unique<QPushButton>		_destroy_input_button;
	Unique<QPushButton>		_create_output_button;
	Unique<QPushButton>		_destroy_output_button;
	Unique<Tree>			_inputs_list;
	Unique<Tree>			_outputs_list;

	// Master volume control port:
	Unique<EventPort>		_master_volume_port;
	Unique<EventPort>		_panic_port;

	// Ports sets:
	InputsMap				_inputs;
	OutputsMap				_outputs;

	// For smoothing master volume:
	AudioBuffer				_master_volume_smoother_buffer;
	DSP::OnePoleSmoother	_master_volume_smoother;
};


inline
Backend::StateChange::StateChange (bool online, bool from_backend):
	QEvent (QEvent::User),
	online (online),
	from_backend (from_backend)
{ }


inline Transport*
Backend::transport() const
{
	return _transport.get();
}


inline EventPort*
Backend::master_volume_port() const
{
	return _master_volume_port.get();
}


inline EventPort*
Backend::panic_port() const
{
	return _panic_port.get();
}

} // namespace AudioBackendImpl

} // namespace Haruhi

#endif

