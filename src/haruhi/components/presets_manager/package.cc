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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/filesystem.h>
#include <haruhi/utility/exception.h>
#include <haruhi/utility/predicates.h>
#include <haruhi/utility/qdom.h>

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
	element.appendChild (meta_el);

	for (Categories::const_iterator c = _categories.begin(); c != _categories.end(); ++c)
	{
		QDomElement cat_el = document.createElement ("category");
		c->save_state (cat_el);
		element.appendChild (cat_el);
	}
}


void
Package::load_state (QDomElement const& element)
{
	_categories.clear();

	for (QDomElement& e: element)
	{
		if (e.tagName() == "meta")
		{
			for (QDomElement& e2: e)
			{
				if (e2.tagName() == "name")
					_name = e2.text();
				else if (e2.tagName() == "version")
					_version = e2.text();
				else if (e2.tagName() == "created-at")
					_created_at = e2.text();
				else if (e2.tagName() == "credits")
					_credits = e2.text();
				else if (e2.tagName() == "license")
					_license = e2.text();
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

