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
#include "input_item.h"
#include "ports_list_view.h"
#include "audio_backend.h"


namespace Haruhi {

namespace AudioBackendPrivate {

InputItem::InputItem (PortsListView* parent, QString const& name):
	PortItem (parent, name)
{
	initialize();
	// Allocate new port:
	_backend->graph()->lock();
	_port = new Core::AudioPort (_backend, name.ascii(), Core::Port::Output);
	_backend->graph()->unlock();
	_backend->_inputs.insert (this);
	// Configure item:
	setIcon (0, Config::Icons16::audio_input_port());
	// Fully constructed:
	set_ready (true);
}


InputItem::~InputItem()
{
	_backend->_inputs.erase (this);
}


void
InputItem::initialize()
{
	// Create JACK input port:
	if (_backend->_jack)
		_jack_port = ::jack_port_register (_backend->_jack, text (0).ascii(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsTerminal | JackPortIsInput, 0);
	// Don't panic if creating failed. It will be a port without external representation (ie. not "valid", but still working).
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
InputItem::transfer()
{
	if (_jack_port)
	{
		Core::AudioBuffer* audio_buffer = _port->audio_buffer();

		void* s = ::jack_port_get_buffer (_jack_port, audio_buffer->size());
		void* d = audio_buffer->begin();

		::memcpy (d, s, sizeof (Core::Sample) * audio_buffer->size());
	}
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

} // namespace AudioBackendPrivate

} // namespace Haruhi

