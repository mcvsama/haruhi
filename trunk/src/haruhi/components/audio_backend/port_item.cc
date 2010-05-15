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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QMessageBox>

// Haruhi:
#include <haruhi/config.h>

// Local:
#include "port_item.h"
#include "ports_list_view.h"
#include "audio_backend.h"


namespace Haruhi {

namespace AudioBackendPrivate {

PortItem::PortItem (PortsListView* parent, QString const& name):
	QTreeWidgetItem (parent, QStringList (name)),
	_backend (parent->backend()),
	_jack_port (0),
	_ready (false)
{
	QSize s = sizeHint (0);
	if (s.height() < 18)
	{
		s.setHeight (18);
		setSizeHint (0, s);
	}
}


PortItem::~PortItem()
{
	_backend->graph()->lock();
	delete _port;
	_backend->graph()->unlock();
	if (_jack_port)
		::jack_port_unregister (_backend->_jack, _jack_port);
	// Remove itself from list view:
	if (parent())
		parent()->takeChild (parent()->indexOfChild (this));
}


QString
PortItem::name() const
{
	return text (0);
}


Core::AudioPort*
PortItem::port() const
{
	return _port;
}


void
PortItem::update_name()
{
	_backend->graph()->lock();
	_port->set_name (text (0).ascii());
	_backend->graph()->unlock();
	if (_jack_port)
		::jack_port_set_name (_jack_port, text (0).ascii());
}


void
PortItem::invalidate()
{
	if (_jack_port)
		::jack_port_unregister (_backend->_jack, _jack_port);
	_jack_port = 0;
}


void
PortItem::offline_invalidate()
{
	_jack_port = 0;
}

} // namespace AudioBackendPrivate

} // namespace Haruhi

