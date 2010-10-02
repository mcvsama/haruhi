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

// Local:
#include "event_transport.h"
#include "port_item.h"
#include "device_item.h"


namespace Haruhi {

namespace EventBackendPrivate {

class ControllerItem: public PortItem
{
	friend class ControllerDialog;

  public:
	ControllerItem (DeviceItem* parent, QString const& name);

	virtual ~ControllerItem();

	void
	learn();

	void
	stop_learning();

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
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  private:
	Core::EventPort*	_port;
	bool				_note_filter;
	int					_note_channel;					// 0 means 'all'
	bool				_controller_filter;
	int					_controller_channel;			// 0 means 'all'
	int					_controller_number;
	bool				_controller_invert;
	bool				_pitchbend_filter;
	int					_pitchbend_channel;				// 0 means 'all'
	bool				_channel_pressure_filter;
	int					_channel_pressure_channel;		// 0 means 'all'
	bool				_channel_pressure_invert;
	bool				_key_pressure_filter;
	int					_key_pressure_channel;			// 0 means 'all'
	bool				_key_pressure_invert;
	// Learning from MIDI mode.
	bool				_learning;
};

} // namespace EventBackendPrivate

} // namespace Haruhi

#endif

