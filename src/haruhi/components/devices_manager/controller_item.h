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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>
#include <haruhi/components/event_backend/transport.h>
#include <haruhi/lib/midi.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "item.h"
#include "controller.h"
#include "device_item.h"


namespace Haruhi {

namespace DevicesManager {

class ControllerItem: public Item
{
  public:
	/**
	 * \param	controller Controller that will be associated with this UI item.
	 */
	ControllerItem (DeviceItem* parent, Controller* controller);

	virtual ~ControllerItem();

	/**
	 * Returns controller name used in Haruhi.
	 */
	QString
	name() const;

	/**
	 * Sets new name for item and controller.
	 */
	void
	set_name (QString const& name);

	/**
	 * Returns Controller object associated with this UI item.
	 */
	Controller*
	controller() const;

	/**
	 * Puts controller into learning mode.
	 * Also changes item icon.
	 */
	void
	learn();

	/**
	 * Resets controller to normal (not learning) mode.
	 */
	void
	stop_learning();

	/**
	 * Returns true if item is in 'learning' mode.
	 */
	bool
	learning();

	/**
	 * If in learning mode, sets filters from MIDI event and stops learning.
	 * \param	force Learns even if not in learning mode.
	 * \returns	true if actually learned from event, false otherwise.
	 */
	bool
	learn_from_event (MIDI::Event const&, bool force = false);

  protected:
	// Controller associated with this UI item; not owned:
	Controller*	_controller;
	// Learning-from-MIDI mode:
	bool		_learning;
};


inline QString
ControllerItem::name() const
{
	return _controller->name();
}


inline Controller*
ControllerItem::controller() const
{
	return _controller;
}


inline bool
ControllerItem::learning()
{
	return _learning;
}

} // namespace DevicesManager

} // namespace Haruhi

#endif

