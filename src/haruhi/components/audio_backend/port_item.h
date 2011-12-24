/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
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
#include <haruhi/graph/audio_port.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "transport.h"


namespace Haruhi {

namespace AudioBackendImpl {

class Backend;
class Tree;

class PortItem:
	public QTreeWidgetItem,
	public SaveableState
{
  public:
	PortItem (Tree* parent, QString const& name);

	~PortItem();

	QString
	name() const;

	Transport::Port*
	transport_port() const;

	AudioPort*
	port() const;

	/**
	 * Tells whether port has been fully constructed and
	 * may be normally used by backend.
	 */
	bool
	ready() const;

	/**
	 * Updates name of backend ports basing on GUI port name.
	 */
	void
	update_name();

	/**
	 * Creates a dialog for port configuration.
	 */
	virtual void
	configure() = 0;

  protected:
	/**
	 * Tell that port is ready to use.
	 * Call from derived class constructor.
	 */
	void
	set_ready (bool r);

  protected:
	Backend*			_backend;
	AudioPort*			_port;
	Transport::Port*	_transport_port;

  private:
	// Set when port is fully constructed:
	bool				_ready;
};


inline Transport::Port*
PortItem::transport_port() const
{
	return _transport_port;
}


inline AudioPort*
PortItem::port() const
{
	return _port;
}


inline bool
PortItem::ready() const
{
	return _ready;
}


inline void
PortItem::set_ready (bool r)
{
	_ready = r;
}

} // namespace AudioBackendImpl

} // namespace Haruhi

#endif

