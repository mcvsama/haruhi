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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__PACKAGE_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__PACKAGE_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>

// Qt:
#include <QtXml/QDomNode>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "category.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

class Package: public SaveableState
{
  public:
	typedef std::list<Category> Categories;

  public:
	Package();

	/**
	 * Package meta-information: name.
	 */
	QString
	name() const { return _name; }

	void
	set_name (QString const& name) { _name = name; }

	/**
	 * Package meta-information: version.
	 */
	QString
	version() const { return _version; }

	void
	set_version (QString const& version) { _version = version; }

	/**
	 * Package meta-information: created_at.
	 */
	QString
	created_at() const { return _created_at; }

	void
	set_created_at (QString const& created_at) { _created_at = created_at; }

	/**
	 * Package meta-information: created_at.
	 */
	QString
	credits() const { return _credits; }

	void
	set_credits (QString const& credits) { _credits = credits; }

	/**
	 * Package meta-information: created_at.
	 */
	QString
	license() const { return _license; }

	void
	set_license (QString const& license) { _license = license; }

	/**
	 * Categories accessor.
	 */
	Categories&
	categories() { return _categories; }

	/**
	 * Categories accessor.
	 */
	Categories const&
	categories() const { return _categories; }

	/**
	 * Creates new child category.
	 */
	Category*
	create_category();

	/**
	 * Removes category from list by its pointer.
	 */
	void
	remove_category (Category* category);

	/**
	 * File name from which this package has been loaded.
	 */
	QString
	file_name() const { return _file_name; }

	void
	set_file_name (QString const& file_name) { _file_name = file_name; }

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement& element) const;

	void
	load_state (QDomElement const& element);

  private:
	Category*
	find_or_create_category (QString const& name);

  private:
	QString			_name;
	QString			_version;
	QString			_created_at;
	QString			_credits;
	QString			_license;
	QString			_file_name;
	Categories		_categories;
};

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

