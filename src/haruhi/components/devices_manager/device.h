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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__DEVICE_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__DEVICE_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>
#include <list>

// Qt:
#include <QString>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/event.h>
#include <haruhi/lib/midi.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "controller.h"


namespace Haruhi {

namespace DevicesManager {

/**
 * Represents event device (usually MIDI device).
 * Contains many controllers.
 */
class Device: public SaveableState
{
	friend class Controller;

  public:
	typedef std::list<Controller> Controllers;

  public:
	Device (QString const& name = "");

	bool
	operator== (Device const& other) const;

	/**
	 * Name for device. Name is for UI.
	 */
	QString
	name() const;

	/**
	 * Sets new name for device.
	 */
	void
	set_name (QString const& name);

	/**
	 * Returns true if auto-add is set on this device.
	 * Auto-add means that device will be added by default to new sessions
	 * in SessionLoader UI.
	 */
	bool
	auto_add() const;

	/**
	 * Sets auto-add property.
	 */
	void
	set_auto_add (bool enabled);

	/**
	 * Accessor to controllers list.
	 */
	Controllers&
	controllers();

	/**
	 * Accessor to controllers list.
	 */
	Controllers const&
	controllers() const;

	/**
	 * Returns true if given Controller points to one of elements in the controllers list.
	 */
	bool
	has_controller (Controller* controller) const;

	/**
	 * Returns iterator pointing to given Controller or past-the-end of the sequence
	 * if Controller is not found.
	 */
	Controllers::iterator
	find_controller (Controller* controller);

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement&) const override;

	void
	load_state (QDomElement const&) override;

  private:
	QString						_name;
	Controllers					_controllers;
	bool						_auto_add;
	std::array<VoiceID, 128>	_voice_ids;
	MIDI::Event::ID				_last_midi_event_id	= 0;
	VoiceID						_allocated_voice_id	= OmniVoice;
};


inline QString
Device::name() const
{
	return _name;
}


inline void
Device::set_name (QString const& name)
{
	_name = name;
}


inline bool
Device::auto_add() const
{
	return _auto_add;
}


inline void
Device::set_auto_add (bool enabled)
{
	_auto_add = enabled;
}


inline Device::Controllers&
Device::controllers()
{
	return _controllers;
}


inline Device::Controllers const&
Device::controllers() const
{
	return _controllers;
}

} // namespace DevicesManager

} // namespace Haruhi

#endif

