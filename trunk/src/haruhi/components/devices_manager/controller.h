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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QString>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/event.h>
#include <haruhi/graph/graph.h>
#include <haruhi/lib/midi.h>
#include <haruhi/utility/saveable_state.h>


namespace Haruhi {

namespace DevicesManager {

/**
 * Represents controller inside device. Used to filter certain MIDI events.
 */
class Controller: public SaveableState
{
  public:
	Controller (QString const& name = "");

	/**
	 * Name for controller. Name is for UI.
	 */
	QString
	name() const { return _name; }

	/**
	 * Sets new name for controller.
	 */
	void
	set_name (QString const& name) { _name = name; }

	/**
	 * Sets filters from MIDI event and stops learning.
	 * \returns	true if learned, false if event was ignored.
	 */
	bool
	learn_from_event (MIDI::Event const& event);

	/**
	 * Interprets MIDI event and maybe adds new Haruhi Events to the buffer.
	 * \param	event Input MIDI event.
	 * \param	buffer Buffer where new events will be added.
	 * \param	graph Graph object for audio/event context (master-tune).
	 * \returns	true if event has been handled, false otherwise.
	 */
	bool
	handle_event (MIDI::Event const& event, EventBuffer& buffer, Graph* graph) const;

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

  private:
	QString	_name;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

