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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Libs:
#include <alsa/asoundlib.h>

// Haruhi:
#include <haruhi/components/event_backend/transport.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "item.h"
#include "device_item.h"


namespace Haruhi {

namespace DevicesManager {

class ControllerItem:
	public Item,
	public SaveableState
{
  public:
	ControllerItem (DeviceItem* parent, QString const& name);

	virtual ~ControllerItem();

	/**
	 * Returns controller name used in Haruhi.
	 */
	QString
	name() const { return QTreeWidgetItem::text (0); }

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
	learning() { return _learning; }

	/**
	 * Sets filters from MIDI event
	 * and stops learning.
	 */
	void
	learn_from_event (EventBackendImpl::Transport::MidiEvent const&);

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  public:
	// MIDI filters:
	bool	note_filter;
	int		note_channel;				// 0 means 'all'
	bool	controller_filter;
	int		controller_channel;			// 0 means 'all'
	int		controller_number;
	bool	controller_invert;
	bool	pitchbend_filter;
	int		pitchbend_channel;			// 0 means 'all'
	bool	channel_pressure_filter;
	int		channel_pressure_channel;	// 0 means 'all'
	bool	channel_pressure_invert;
	bool	key_pressure_filter;
	int		key_pressure_channel;		// 0 means 'all'
	bool	key_pressure_invert;

  protected:
	// Learning from MIDI mode:
	bool	_learning;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

