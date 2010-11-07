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

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

	// TODO learn from MIDI
	void
	learn() { }

  public:
	bool	_note_filter;
	int		_note_channel;					// 0 means 'all'
	bool	_controller_filter;
	int		_controller_channel;			// 0 means 'all'
	int		_controller_number;
	bool	_controller_invert;
	bool	_pitchbend_filter;
	int		_pitchbend_channel;				// 0 means 'all'
	bool	_channel_pressure_filter;
	int		_channel_pressure_channel;		// 0 means 'all'
	bool	_channel_pressure_invert;
	bool	_key_pressure_filter;
	int		_key_pressure_channel;			// 0 means 'all'
	bool	_key_pressure_invert;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

