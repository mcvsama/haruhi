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

// Haruhi:
#include <haruhi/config.h>

// Local:
#include "output_item.h"
#include "ports_list_view.h"
#include "audio_backend.h"


namespace Haruhi {

namespace AudioBackendPrivate {

OutputItem::OutputItem (PortsListView* parent, QString const& name):
	PortItem (parent, name)
{
	_transport_port = _backend->transport()->create_output (name.toStdString());
	// Allocate new port:
	_backend->graph()->lock();
	_port = new Core::AudioPort (_backend, name.ascii(), Core::Port::Input);
	_backend->graph()->unlock();
	_backend->_outputs[_transport_port] = this;
	// Configure item:
	setIcon (0, Config::Icons16::audio_output_port());
	graph_updated();
	// Fully constructed:
	set_ready (true);
}


OutputItem::~OutputItem()
{
	_backend->_outputs.erase (_transport_port);
	_backend->transport()->destroy_port (_transport_port);
	_backend->graph()->lock();
	delete _port;
	_backend->graph()->unlock();
	// Remove itself from External ports list view:
	if (treeWidget())
		treeWidget()->invisibleRootItem()->takeChild (treeWidget()->invisibleRootItem()->indexOfChild (this));
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


void
OutputItem::graph_updated()
{
	// 100ms:
	_smoother.set_smoothing_samples (0.100f * _backend->graph()->sample_rate());
}


void
OutputItem::attenuate (Core::Sample value)
{
	_smoother.set_value (value);
	_smoother.multiply (port()->audio_buffer()->begin(), port()->audio_buffer()->end());
}

} // namespace AudioBackendPrivate

} // namespace Haruhi

