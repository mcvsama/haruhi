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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "input_item.h"
#include "tree.h"
#include "backend.h"


namespace Haruhi {

namespace AudioBackendImpl {

InputItem::InputItem (Tree* parent, QString const& name):
	PortItem (parent, name)
{
	_transport_port = _backend->transport()->create_input (name.toStdString());
	// Allocate new port:
	_backend->graph()->lock();
	_port = new AudioPort (_backend, name.ascii(), Port::Output);
	_backend->graph()->unlock();
	_backend->_ports_lock.lock();
	_backend->_inputs[_transport_port] = this;
	_backend->_ports_lock.unlock();
	// Configure item:
	setIcon (0, Resources::Icons16::audio_input_port());
	// Fully constructed:
	set_ready (true);
}


InputItem::~InputItem()
{
	_backend->_ports_lock.lock();
	_backend->_inputs.erase (_transport_port);
	_backend->_ports_lock.unlock();
	_backend->transport()->destroy_port (_transport_port);
	_backend->graph()->lock();
	delete _port;
	_backend->graph()->unlock();
	// Remove itself from External ports list view:
	if (treeWidget())
		treeWidget()->invisibleRootItem()->takeChild (treeWidget()->invisibleRootItem()->indexOfChild (this));
}


void
InputItem::configure()
{
	InputDialog* dialog = new InputDialog (_backend, _backend);
	dialog->from (this);
	if (dialog->exec() == InputDialog::Accepted)
		dialog->apply (this);
}


void
InputItem::save_state (QDomElement& element) const
{
	element.setAttribute ("name", name());
}


void
InputItem::load_state (QDomElement const& element)
{
	setText (0, element.attribute ("name"));
	update_name();
}

} // namespace AudioBackendImpl

} // namespace Haruhi

