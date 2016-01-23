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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/widgets/generic_item.h>

// Local:
#include "port_item.h"
#include "tree.h"
#include "backend.h"


namespace Haruhi {

namespace AudioBackendImpl {

PortItem::PortItem (Tree* parent, QString const& name):
	QTreeWidgetItem (parent, QStringList (name)),
	_backend (parent->backend()),
	_port (0),
	_transport_port (0),
	_ready (false)
{
	make_standard_height (this);
}


PortItem::~PortItem()
{
	// Remove itself from TreeWidget:
	if (parent())
		parent()->removeChild (this);
}


QString
PortItem::name() const
{
	return text (0);
}


void
PortItem::update_name()
{
	_transport_port->rename (name().toStdString());
	Mutex::Lock lock (*_backend->graph());
	_port->set_name (name().toStdString());
}

} // namespace AudioBackendImpl

} // namespace Haruhi

