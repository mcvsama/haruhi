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

// Qt:
#include <QtCore/QFile>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "package.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

Category::Category()
{
}


void
Category::save_state (QDomElement& element) const
{
	element.setAttribute ("name", _name);

	for (Presets::const_iterator p = _presets.begin(); p != _presets.end(); ++p)
	{
		QDomElement preset_el = element.ownerDocument().createElement ("preset");
		p->save_state (preset_el);
		element.appendChild (preset_el);
	}
}


void
Category::load_state (QDomElement const& element)
{
	_presets.clear();

	set_name (element.attribute ("name", "<unnamed category>"));

	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (e.isNull())
			continue;
		else if (e.tagName() == "preset")
		{
			_presets.push_back (Preset());
			_presets.back().load_state (e);
		}
	}
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

