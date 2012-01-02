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

#ifndef HARUHI__SETTINGS__HARUHI_SETTINGS_H__INCLUDED
#define HARUHI__SETTINGS__HARUHI_SETTINGS_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>
#include <map>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "settings.h"


namespace Haruhi {

class HaruhiSettings: public Settings::Module
{
  public:
	HaruhiSettings();

	int
	engine_thread_priority() const;

	void
	set_engine_thread_priority (int value);

	int
	level_meter_fps() const;

	void
	set_level_meter_fps (int value);

	/*
	 * Settings::Module API
	 */

	void
	load_state (QDomElement const& element);

	void
	save_state (QDomElement& element) const;

  private:
	int	_engine_thread_priority;
	int	_level_meter_fps;
};


inline int
HaruhiSettings::engine_thread_priority() const
{
	return _engine_thread_priority;
}


inline void
HaruhiSettings::set_engine_thread_priority (int value)
{
	_engine_thread_priority = value;
}


inline int
HaruhiSettings::level_meter_fps() const
{
	return _level_meter_fps;
}


inline void
HaruhiSettings::set_level_meter_fps (int value)
{
	_level_meter_fps = value;
}

} // namespace Haruhi

#endif

