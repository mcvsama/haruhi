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

// Local:
#include "output_item.h"
#include "tree.h"
#include "backend.h"


namespace Haruhi {

namespace AudioBackendImpl {

OutputItem::OutputItem (Tree* parent, QString const& name):
	PortItem (parent, name)
{
	_transport_port = _backend->transport()->create_output (name.toStdString());
	// Allocate new port:
	_backend->graph()->synchronize ([&] {
		_port = new AudioPort (_backend, name.ascii(), Port::Input);
	});
	_backend->_ports_lock.synchronize ([&] {
		_backend->_outputs[_transport_port] = this;
	});
	// Configure item:
	setIcon (0, Resources::Icons16::audio_output_port());
	// Fully constructed:
	set_ready (true);
}


OutputItem::~OutputItem()
{
	_backend->_ports_lock.synchronize ([&] {
		_backend->_outputs.erase (_transport_port);
	});
	_backend->transport()->destroy_port (_transport_port);
	_backend->graph()->synchronize ([&] {
		delete _port;
	});
	// Remove itself from External ports list view:
	if (treeWidget())
		treeWidget()->invisibleRootItem()->removeChild (this);
}


void
OutputItem::configure()
{
	OutputDialog* dialog = new OutputDialog (_backend, _backend);
	dialog->from (this);
	if (dialog->exec() == OutputDialog::Accepted)
		dialog->apply (this);
}


void
OutputItem::save_state (QDomElement& element) const
{
	element.setAttribute ("name", name());
}


void
OutputItem::load_state (QDomElement const& element)
{
	setText (0, element.attribute ("name"));
	update_name();
}

} // namespace AudioBackendImpl

} // namespace Haruhi

