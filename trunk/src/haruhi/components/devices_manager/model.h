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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__MODEL_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__MODEL_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/signal.h>

// Local:
#include "device.h"


namespace Haruhi {

namespace DevicesManager {

/**
 * Top container for Devices.
 */
class Model: public SaveableState
{
  public:
	typedef std::list<Device> Devices;

  public:
	Model();

	/**
	 * Devices accessor.
	 */
	Devices&
	devices() { return _devices; }

	/**
	 * Devices accessor.
	 */
	Devices const&
	devices() const { return _devices; }

	/**
	 * Should be called when model changes, ie. devices/controllers are added or removed,
	 * or their properties change. Emits on_change() signal.
	 */
	void
	changed();

	/**
	 * Returns true if given Device points to one of the elements in controllers list.
	 */
	bool
	has_device (Device* device) const;

	/**
	 * Returns true if there is at least one device with given name.
	 */
	bool
	has_device_named (QString const& name) const;

	/**
	 * Returns iterator pointing to given Device or past-the-end of the sequence
	 * if Device is not found.
	 */
	Devices::iterator
	find_device (Device* device);

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement& element) const;

	void
	load_state (QDomElement const& element);

  public:
	// Emited when model_changed() is called:
	Signal::Emiter0	on_change;

  private:
	Devices	_devices;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

