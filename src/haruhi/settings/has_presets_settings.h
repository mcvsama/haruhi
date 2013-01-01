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

#ifndef HARUHI__SETTINGS__HAS_PRESETS_SETTINGS_H__INCLUDED
#define HARUHI__SETTINGS__HAS_PRESETS_SETTINGS_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <set>
#include <map>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "settings.h"


namespace Haruhi {

class HasPresetsSettings: public Settings::Module
{
  public:
	// Set of preset UUIDs marked by user as favorite:
	typedef std::set<std::string>					FavoritePresets;
	// Maps Unit URNs to set of favorite presets:
	typedef std::map<std::string, FavoritePresets>	Units;

  public:
	HasPresetsSettings();

	bool
	favorited (std::string const& unit_urn, std::string const& preset_uuid) const;

	void
	set_favorited (std::string const& unit_urn, std::string const& preset_uuid, bool set);

	FavoritePresets&
	favorite_presets_for_unit (std::string const& unit_urn);

	/*
	 * Settings::Module API
	 */

	void
	save_state (QDomElement& element) const override;

	void
	load_state (QDomElement const& element) override;

  private:
	Units _units;
};

} // namespace Haruhi

#endif

