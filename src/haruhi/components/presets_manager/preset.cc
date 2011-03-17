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

// System:
#include <uuid/uuid.h>

// Qt:
#include <QtCore/QFile>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "package.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

Preset::Preset()
{
	generate_uuid();
}


void
Preset::clear_patch_element (QDomDocument& document)
{
	_patch = document.createElement ("patch");
}


void
Preset::save_state (QDomElement& element) const
{
	element.setAttribute ("uuid", _uuid);
	QDomElement meta_el= element.ownerDocument().createElement ("meta");
	append_element (meta_el, "name", _name);
	append_element (meta_el, "version", _version);
	append_element (meta_el, "created-at", _created_at);
	element.appendChild (meta_element);
	element.appendChild (_patch.cloneNode (true));
}


void
Preset::load_state (QDomElement const& element)
{
	_uuid = element.attribute ("uuid");
	if (_uuid.isEmpty() || _uuid.isNull())
		generate_uuid();
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (e.isNull())
			continue;
		else if (e.tagName() == "meta")
		{
			for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
			{
				QDomElement e = n.toElement();
				if (e.isNull())
					continue;
				else if (e.tagName() == "name")
					_name = e.text();
				else if (e.tagName() == "version")
					_version = e.text();
				else if (e.tagName() == "created-at")
					_created_at = e.text();
			}
		}
		else if (e.tagName() == "patch")
			_patch = e.cloneNode (true).toElement();
	}
}


void
Preset::generate_uuid()
{
	char uuid_string[37];
	uuid_t uuid;
	::uuid_generate (uuid);
	::uuid_unparse_lower (uuid, uuid_string);
	_uuid = QString (uuid_string);
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

