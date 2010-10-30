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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__CONTROLLER_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__CONTROLLER_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Libs:
#include <alsa/asoundlib.h>

// Haruhi:
#include <haruhi/core/event_port.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/components/devices_manager/controller_item.h>

// Local:
#include "event_transport.h"
#include "port_item.h"
#include "device_with_port_item.h"


namespace Haruhi {

namespace EventBackendPrivate {

using DevicesManagerPrivate::ControllerItem;

class ControllerWithPortItem:
	public ControllerItem,
	public PortItem
{
	friend class ControllerDialog;

  public:
	ControllerWithPortItem (DeviceWithPortItem* parent, QString const& name);

	virtual ~ControllerWithPortItem();

	/**
	 * Puts controller into learning mode.
	 * Uses EventBackend learning methods to setup controller
	 * according to first incoming MIDI event.
	 */
	void
	learn();

	/**
	 * Resets controller to normal (not learning) mode.
	 */
	void
	stop_learning();

	/**
	 * Sets Core port's name to what
	 * was set by user in the UI.
	 */
	void
	update_name();

	QString
	name() const;

	Core::EventPort*
	port() const;

	/**
	 * Create and push new Event into core graph.
	 * Returns true if event has been actually passed by port.
	 */
	bool
	handle_event (EventTransport::MidiEvent const& event);

	void
	load_state (QDomElement const& element);

  private:
	Core::EventPort*	_port;
	// Link to DeviceItem:
	DeviceWithPortItem*	_device_item;
	// Learning from MIDI mode.
	bool				_learning;
};

} // namespace EventBackendPrivate

} // namespace Haruhi

#endif

