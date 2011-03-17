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
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QTreeWidgetItemIterator>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/filesystem.h>
#include <haruhi/utility/exception.h>

// Local:
#include "helpers.h"
#include "package_item.h"
#include "category_item.h"
#include "preset_item.h"
#include "presets_tree.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

void
PackageItem::Meta::save_state (QDomElement& element) const
{
	append_element (element, "name", name);
	append_element (element, "version", version);
	append_element (element, "created-at", created_at);
	append_element (element, "credits", credits);
	append_element (element, "license", license);
}


void
PackageItem::Meta::load_state (QDomElement const& element)
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
			else if (e.tagName() == "credits")
				credits = e.text();
			else if (e.tagName() == "license")
				license = e.text();
		}
	}
}


PackageItem::PackageItem (PresetsTree* parent):
	QTreeWidgetItem (parent),
	_presets_manager (parent->presets_manager())
{
	setup();
}


void
PackageItem::setup()
{
	setIcon (0, Resources::Icons16::presets_package());
	QSize s = sizeHint (0);
	if (s.height() < 18)
	{
		s.setHeight (18);
		setSizeHint (0, s);
	}
}


void
PackageItem::reload()
{
	setText (0, _meta.name);
}


void
PackageItem::load_file (QString const& file_name)
{
	_file_name = file_name;

	QFile file (file_name);
	if (file.open (IO_ReadOnly))
	{
		_document.setContent (&file, true);
		file.close();

		QDomElement root_element = _document.documentElement();
		if (root_element.tagName() == "haruhi-presets")
		{
			_unit_urn = root_element.attribute ("unit");
			load_state (root_element);
		}
	}
	else
		throw Exception (QString ("Could not read presets package file '%1'.").arg (file_name));
}


void
PackageItem::save_file()
{
	mkpath (presets_manager()->model()->directory().toStdString(), 0700);

	QString name = meta().name;
	name.replace ('/', "_");
	QString file_name = presets_manager()->model()->directory() + "/" + name + ".haruhi-presets";

	QString to_delete;
	if (_file_name != file_name)
		to_delete = _file_name;
	_file_name = file_name;

	// Create DOM document:
	QDomDocument document;
	QDomElement root_element = document.createElement ("haruhi-presets");
	root_element.setAttribute ("unit", _unit_urn);
	save_state (root_element);
	document.appendChild (root_element);

	// Save file:
	QFile file (_file_name + "~");
	if (!file.open (IO_WriteOnly))
		throw Exception (QString ("Could not save package: ") + file.errorString());
	QTextStream ts (&file);
	ts << document.toString();
	file.flush();
	file.close();
	::rename (_file_name + "~", _file_name);

	if (to_delete != QString::null)
		QFile::remove (to_delete);
}


void
PackageItem::remove_file()
{
	QFile::remove (_file_name);
}


void
PackageItem::save_state (QDomElement& element) const
{
	QDomDocument document = element.ownerDocument();
	QDomElement meta_element = document.createElement ("meta");
	_meta.save_state (meta_element);
	element.appendChild (meta_element);

	for (int i = 0; i < childCount(); ++i)
	{
		CategoryItem* category_item = dynamic_cast<CategoryItem*> (child (i));
		if (category_item)
		{
			QDomElement presets_element = document.createElement ("presets");
			presets_element.setAttribute ("category", category_item->name());

			for (int j = 0; j < category_item->childCount(); ++j)
			{
				PresetItem* preset_item = dynamic_cast<PresetItem*> (category_item->child (j));
				if (preset_item)
				{
					QDomElement preset_element = document.createElement ("preset");
					preset_item->ensure_has_uuid();
					preset_item->save_state (preset_element);
					presets_element.appendChild (preset_element);
				}
			}
			element.appendChild (presets_element);
		}
	}
}


void
PackageItem::load_state (QDomElement const& element)
{
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "meta")
			{
				_meta.load_state (e);
				reload();
			}
			else if (e.tagName() == "presets")
			{
				CategoryItem* category_item = find_or_create_category (e.attribute ("category"));

				for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
				{
					QDomElement e = n.toElement();
					if (!e.isNull())
					{
						if (e.tagName() == "preset")
						{
							PresetItem* preset_item = new PresetItem (category_item);
							preset_item->load_state (e);
						}
					}
				}
			}
		}
	}
}


CategoryItem*
PackageItem::find_or_create_category (QString const& name)
{
	for (QTreeWidgetItemIterator item (this, QTreeWidgetItemIterator::NoChildren); *item; ++item)
	{
		CategoryItem* category_item = dynamic_cast<CategoryItem*> (*item);
		if (category_item && category_item->name() == name)
			return category_item;
	}
	return new CategoryItem (name, this);
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

