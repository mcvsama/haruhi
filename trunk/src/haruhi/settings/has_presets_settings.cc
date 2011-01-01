/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
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


namespace Haruhi {

HasPresetsSettings::HasPresetsSettings():
	Module ("has-presets")
{
}


void
HasPresetsSettings::save_state (QDomElement& element) const
{
	for (Units::const_iterator u = _units.begin(); u != _units.end(); ++u)
	{
		QDomElement unit_el = element.ownerDocument().createElement ("unit");
		unit_el.setAttribute ("urn", u->first.c_str());
		for (FavoritePresets::const_iterator fp = u->second.begin(); fp != u->second.end(); ++fp)
		{
			QDomElement fp_el = element.ownerDocument().createElement ("favorite-preset");
			fp_el.setAttribute ("uuid", fp->c_str());
			unit_el.appendChild (fp_el);
		}
		element.appendChild (unit_el);
	}
}


void
HasPresetsSettings::load_state (QDomElement const& element)
{
	_units.clear();

	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (e.isNull())
			continue;
		if (e.tagName() == "unit")
		{
			QString unit_urn = e.attribute ("urn");
			if (!unit_urn.isEmpty())
			{
				FavoritePresets& favorite_presets = _units[unit_urn.toStdString()];
				for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
				{
					QDomElement e = n.toElement();
					if (e.isNull())
						continue;
					QString fp_uuid = e.attribute ("uuid");
					if (!fp_uuid.isEmpty())
						favorite_presets.insert (fp_uuid.toStdString());
				}
			}
		}
	}
}


HasPresetsSettings::FavoritePresets&
HasPresetsSettings::favorite_presets_for_unit (std::string const& unit_urn)
{
	return _units[unit_urn];
}

} // namespace Haruhi
