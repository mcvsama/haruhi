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
#include <libgen.h>

// Qt:
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtXml/QDomNode>
#include <QtGui/QTextDocument>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/filesystem.h>
#include <haruhi/utility/exception.h>
#include <haruhi/utility/predicates.h>

// Local:
#include "helpers.h"
#include "package.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

Package::Package():
	_name ("<new package>")
{
}


Category*
Package::create_category()
{
	_categories.push_back (Category());
	return &_categories.back();
}


void
Package::remove_category (Category* category)
{
	_categories.remove_if (PointerEquals<Category> (category));
}


QString
Package::file_name() const
{
	return name().replace ('/', "_") + ".haruhi-presets";
}


void
Package::remove_file()
{
	if (_loaded_file_name != QString::null)
		QFile::remove (_loaded_file_name);
}


void
Package::load_file (QString const& path)
{
	QFile file (path);
	if (file.open (IO_ReadOnly))
	{
		_document.setContent (&file, true);
		file.close();

		QDomElement root_element = _document.documentElement();
		if (root_element.tagName() == "haruhi-presets")
		{
			_unit_urn = root_element.attribute ("unit");
			load_state (root_element);
			_loaded_file_name = path;
		}
	}
	else
		throw Exception (QString ("Could not read presets package file '%1'.").arg (Qt::escape (path)));
}


void
Package::save_file (QString const& path)
{
	char* copy = strdup (path.utf8());
	char* dir = dirname (copy);
	mkpath (dir, 0700);
	free (copy);

	QString to_delete;
	QString fn;
	if (_loaded_file_name != QString::null && _loaded_file_name != file_name())
		to_delete = _loaded_file_name;
	fn = file_name();

	// Create DOM document:
	QDomDocument document;
	QDomElement root_element = document.createElement ("haruhi-presets");
	root_element.setAttribute ("unit", _unit_urn);
	save_state (root_element);
	document.appendChild (root_element);

	// Save file:
	QFile file (fn+ "~");
	if (!file.open (IO_WriteOnly))
		throw Exception (QString ("Could not save package: ") + file.errorString());
	QTextStream ts (&file);
	ts << document.toString();
	file.flush();
	file.close();
	::rename (fn + "~", fn);
	_loaded_file_name = fn;

	if (to_delete != QString::null)
		QFile::remove (to_delete);
}


void
Package::save_file()
{
	save_file (_loaded_file_name);
}


void
Package::save_state (QDomElement& element) const
{
	QDomDocument document = element.ownerDocument();

	QDomElement meta_el = document.createElement ("meta");
	append_element (meta_el, "name", _name);
	append_element (meta_el, "version", _version);
	append_element (meta_el, "created-at", _created_at);
	append_element (meta_el, "credits", _credits);
	append_element (meta_el, "license", _license);

	QDomElement categories_el = document.createElement ("categories");

	for (Categories::const_iterator c = _categories.begin(); c != _categories.end(); ++c)
	{
		QDomElement cat_el = document.createElement ("category");
		c->save_state (cat_el);
		categories_el.appendChild (cat_el);
	}

	element.appendChild (meta_el);
	element.appendChild (categories_el);
}


void
Package::load_state (QDomElement const& element)
{
	_categories.clear();

	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (e.isNull())
			continue;
		else if (e.tagName() == "meta")
		{
			for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
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
				else if (e.tagName() == "credits")
					_credits = e.text();
				else if (e.tagName() == "license")
					_license = e.text();
			}
		}
		else if (e.tagName() == "category")
		{
			_categories.push_back (Category());
			_categories.back().load_state (e);
		}
	}
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

