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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__PORT_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__PORT_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Libs:
#include <jack/jack.h>

// Qt:
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/core/audio_port.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/atomic.h>


namespace Haruhi {

class AudioBackend;

namespace AudioBackendPrivate {

class PortsListView;

class PortItem:
	public QTreeWidgetItem,
	public SaveableState
{
  public:
	struct CompareByPortName
	{
		bool
		operator() (PortItem* a, PortItem* b) const
		{
			return a->port()->name() < b->port()->name();
		}
	};

  public:
	PortItem (PortsListView* parent, QString const& name);

	~PortItem();

	QString
	name() const;

	Core::AudioPort*
	port() const;

	/**
	 * Tells if output (JACK) port is valid.
	 */
	bool
	valid() const { return _jack_port != 0; }

	/**
	 * Tells whether port has been fully constructed and
	 * may be normally used by backend.
	 */
	bool
	ready() const { return _ready; }

	/**
	 * Invalidates port, ie destroys JACK port.
	 */
	void
	invalidate();

	/**
	 * Invalidates port (drops JACK port info), but does not
	 * try to destroy JACK port (use it if jack is already disconnected).
	 */
	void
	offline_invalidate();

	/**
	 * Updates name of backend ports basing on GUI port name.
	 */
	void
	update_name();

	/**
	 * Creates JACK port.
	 */
	virtual void
	initialize() = 0;

	/**
	 * Creates a dialog for port configuration.
	 */
	virtual void
	configure() = 0;

	/**
	 * Transfers data between JACK port and internal port, or vice versa.
	 */
	virtual void
	transfer() = 0;

  protected:
	void
	set_ready (bool r) { atomic (_ready) = r; }

  protected:
	AudioBackend*		_backend;
	QString				_before_rename_text;
	Core::AudioPort*	_port;
	::jack_port_t*		_jack_port;
	bool				_ready;
};

} // namespace AudioBackendPrivate

} // namespace Haruhi

#endif

