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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__AUDIO_BACKEND_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__AUDIO_BACKEND_H__INCLUDED

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

// Libs:
#include <jack/jack.h>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/haruhi.h>
#include <haruhi/core/audio.h>
#include <haruhi/core/event_port.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/backend.h>
#include <haruhi/unit.h>

// Local:
#include "ports_list_view.h"
#include "port_item.h"
#include "input_item.h"
#include "output_item.h"
#include "port_dialog.h"
#include "input_dialog.h"
#include "output_dialog.h"
#include "dummy_thread.h"


namespace Haruhi {

class AudioBackend:
	public Unit,
	public SaveableState,
	public Backend
{
	Q_OBJECT

	class OfflineNotification: public QEvent
	{
	  public:
		OfflineNotification():
			QEvent (QEvent::User)
		{ }
	};

	friend class AudioBackendPrivate::PortItem;
	friend class AudioBackendPrivate::InputItem;
	friend class AudioBackendPrivate::OutputItem;

  private:
	typedef std::set<AudioBackendPrivate::InputItem*>	InputsSet;
	typedef std::set<AudioBackendPrivate::OutputItem*>	OutputsSet;

  public:
	AudioBackend (Session* session, QString const& client_name, int id, QWidget* parent);

	virtual ~AudioBackend();

	/**
	 * Starts processing.
	 */
	void
	enable();

	/**
	 * Stops processing. AudioBackend must not enter processing round after this call ends.
	 * Otherwise system behavior is undefined.
	 */
	void
	disable();

	/**
	 * \returns	backend sample rate.
	 */
	std::size_t
	sample_rate();

	/**
	 * \returns	backend buffer size.
	 */
	std::size_t
	buffer_size();

	/**
	 * Haruhi::Unit methods:
	 */

	void
	process();

	/**
	 * SaveableState methods:
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

	/**
	 * Backend methods:
	 */

	bool
	connected() const;

  public slots:
	void
	connect();

	void
	disconnect();

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
	/**
	 * Runs dummy graph iteration.
	 * Should not be called when audio-backend is normally running.
	 */
	void
	dummy_round();

  protected:
	void
	customEvent (QEvent* event);

	void
	graph_updated();

  private:
	void
	update_graph();

	void
	ignore_sigpipe();

	void
	invalidate_ports();

	void
	offline_invalidate_ports();

	void
	jack_disconnected();

	/**
	 * Main process callback.
	 */
	int
	cb_process (jack_nframes_t nframes);

	/**
	 * Called when changing sample rate.
	 */
	int
	cb_sample_rate_change (jack_nframes_t nframes);

	/**
	 * Called when changing buffer sizes.
	 */
	int
	cb_buffer_size_change (jack_nframes_t nframes);

	/**
	 * Called when JACKit daemon shuts down.
	 */
	void
	cb_shutdown();

	/**
	 * Main process callback.
	 */
	static int
	static_cb_process (jack_nframes_t nframes, void* klass);

	/**
	 * Called when changing sample rate.
	 */
	static int
	static_cb_sample_rate_change (jack_nframes_t nframes, void* klass);

	/**
	 * Called when changing buffer sizes.
	 */
	static int
	static_cb_buffer_size_change (jack_nframes_t nframes, void* klass);

	/**
	 * Called when JACKit daemon shuts down.
	 */
	static void
	static_cb_shutdown (void* klass);

  private:
	// JACK:
	QString				_client_name;
	::jack_client_t*	_jack;
	std::size_t			_sample_rate;
	std::size_t			_buffer_size;

	// Views:
	QPushButton*		_disconnect_button;
	QPushButton*		_reconnect_button;
	QPushButton*		_create_input_button;
	QPushButton*		_destroy_input_button;
	QPushButton*		_create_output_button;
	QPushButton*		_destroy_output_button;

	// Ports lists:
	AudioBackendPrivate::PortsListView*	_inputs_list;
	AudioBackendPrivate::PortsListView*	_outputs_list;

	// Master volume control port:
	Core::EventPort*	_master_volume_port;
	Core::EventPort*	_panic_port;
	bool				_panic_pressed;

	// Used for graph ticks when audio backend is not running:
	QTimer*				_dummy_timer;

	// Ports sets:
	InputsSet			_inputs;
	OutputsSet			_outputs;

	// Other:
	AudioBackendPrivate::DummyThread _dummy_thread;
};


/**
 * AudioBackendException
 */
class AudioBackendException: public Exception
{
  public:
	explicit AudioBackendException (const char* what, const char* details):
		Exception (what, details)
	{ }
};


/**
 * AudioBackendPortException
 */
class AudioBackendPortException: public AudioBackendException
{
  public:
	explicit AudioBackendPortException (const char* what, const char* details):
		AudioBackendException (what, details)
	{ }
};

} // namespace Haruhi

#endif

