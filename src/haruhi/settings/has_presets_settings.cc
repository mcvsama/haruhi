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

// Standard:
#include <cstddef>

// Local:
#include "has_presets_settings.h"

// Haruhi:
#include <haruhi/utility/qdom.h>


namespace Haruhi {

HasPresetsSettings::HasPresetsSettings():
	Module ("has-presets")
{
}


bool
HasPresetsSettings::favorited (std::string const& unit_urn, std::string const& preset_uuid) const
{
	Units::const_iterator u = _units.find (unit_urn);
	if (u == _units.end())
		return false;
	FavoritePresets::const_iterator f = u->second.find (preset_uuid);
	return f != u->second.end();
}


void
HasPresetsSettings::set_favorited (std::string const& unit_urn, std::string const& preset_uuid, bool set)
{
	if (set)
		_units[unit_urn].insert (preset_uuid);
	else
		_units[unit_urn].erase (preset_uuid);
}


HasPresetsSettings::FavoritePresets&
HasPresetsSettings::favorite_presets_for_unit (std::string const& unit_urn)
{
	return _units[unit_urn];
}


void
HasPresetsSettings::save_state (QDomElement& element) const
{
	for (auto u: _units)
	{
		if (u.second.empty())
			continue;
		QDomElement unit_el = element.ownerDocument().createElement ("unit");
		unit_el.setAttribute ("urn", u.first.c_str());
		for (auto& fp: u.second)
		{
			QDomElement fp_el = element.ownerDocument().createElement ("favorite-preset");
			fp_el.setAttribute ("uuid", fp.c_str());
			unit_el.appendChild (fp_el);
		}
		element.appendChild (unit_el);
	}
}


void
HasPresetsSettings::load_state (QDomElement const& element)
{
	_units.clear();

	for (QDomElement& e: element)
	{
		if (e.tagName() == "unit")
		{
			QString unit_urn = e.attribute ("urn");
			if (!unit_urn.isEmpty())
			{
				FavoritePresets& favorite_presets = _units[unit_urn.toStdString()];
				for (QDomElement& e2: e)
				{
					QString fp_uuid = e2.attribute ("uuid");
					if (!fp_uuid.isEmpty())
						favorite_presets.insert (fp_uuid.toStdString());
				}
			}
		}
	}
}

} // namespace Haruhi
