/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__CONTROLLER_WITH_PORT_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__CONTROLLER_WITH_PORT_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Libs:
#include <alsa/asoundlib.h>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/components/devices_manager/controller_item.h>
#include <haruhi/components/devices_manager/controller_dialog.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/lib/midi.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "port_item.h"
#include "device_with_port_item.h"
#include "transport.h"


namespace Haruhi {

namespace EventBackendImpl {

using DevicesManager::ControllerItem;
using DevicesManager::ControllerDialog;

class ControllerWithPortItem:
	public ControllerItem,
	public PortItem
{
  public:
	ControllerWithPortItem (DeviceWithPortItem* parent, DevicesManager::Controller* controller);

	virtual ~ControllerWithPortItem();

	EventPort*
	port() const;

	/**
	 * Sets Core port's name to what was set by user in the UI.
	 */
	void
	update_name();

	/**
	 * Create and push new Event into core graph.
	 * Returns true if event has been actually passed by port.
	 */
	bool
	handle_event (MIDI::Event const& event, DeviceWithPortItem& device);

	/**
	 * Create and push ControllerEvents used for value smoothing.
	 * This method should be called on each processing round.
	 */
	void
	generate_smoothing_events();

  private:
	EventPort*			_port;
	// Link to DeviceItem:
	DeviceWithPortItem*	_device_item;
};


inline EventPort*
ControllerWithPortItem::port() const
{
	return _port;
}

} // namespace EventBackendImpl

} // namespace Haruhi

#endif

