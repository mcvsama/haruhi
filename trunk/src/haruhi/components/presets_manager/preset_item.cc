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

// Standard:
#include <cstddef>

// System:
#include <uuid/uuid.h>

// Qt:
#include <Qt3Support/Q3ListView>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "category_item.h"
#include "preset_item.h"
#include "presets_list_view.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

void
PresetItem::Meta::save_state (QDomElement& element) const
{
	PresetsManager::append_element (element, "name", name);
	PresetsManager::append_element (element, "version", version);
}


void
PresetItem::Meta::load_state (QDomElement const& element)
{
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "name")
				name = e.text();
			else if (e.tagName() == "version")
				version = e.text();
			else if (e.tagName() == "created-at")
				created_at = e.text();
		}
	}
}


PresetItem::PresetItem (CategoryItem* parent):
	Q3ListViewItem (parent)
{
	setPixmap (0, Resources::Icons16::preset());
	setDragEnabled (true);
}


CategoryItem*
PresetItem::category_item() const
{
	Q3ListViewItem* p = parent();
	if (p)
	{
		CategoryItem* c = dynamic_cast<CategoryItem*> (p);
		if (c)
			return c;
	}
	return 0;
}


void
PresetItem::reload()
{
	setText (0, _meta.name);
}


void
PresetItem::clear_patch_element (QDomDocument& document)
{
	_patch = document.createElement ("patch");
}


void
PresetItem::ensure_has_uuid()
{
	// Generate UUID if this is a new preset:
	if (_uuid == QString::null || _uuid.isEmpty())
		generate_uuid();
}


void
PresetItem::save_state (QDomElement& element) const
{
	element.setAttribute ("uuid", _uuid);
	QDomElement meta_element = element.ownerDocument().createElement ("meta");
	_meta.save_state (meta_element);
	element.appendChild (meta_element);
	element.appendChild (_patch.cloneNode (true));
}


void
PresetItem::load_state (QDomElement const& element)
{
	_uuid = element.attribute ("uuid");
	ensure_has_uuid();
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "meta")
				_meta.load_state (e);
			else if (e.tagName() == "patch")
				_patch = e.cloneNode (true).toElement();
		}
	}
	reload();
}


void
PresetItem::generate_uuid()
{
	char uuid_string[37];
	uuid_t uuid;
	::uuid_generate (uuid);
	::uuid_unparse_lower (uuid, uuid_string);
	_uuid = QString (uuid_string);
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

