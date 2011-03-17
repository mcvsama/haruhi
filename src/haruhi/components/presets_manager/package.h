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

	/**
	 * Package meta-information: version.
	 */
	QString
	version() const { return _version; }

	/**
	 * Package meta-information: created_at.
	 */
	QString
	created_at() const { return _created_at; }

	/**
	 * Package meta-information: created_at.
	 */
	QString
	credits() const { return _credits; }

	/**
	 * Package meta-information: created_at.
	 */
	QString
	license() const { return _license; }

	/**
	 * Returns base name of the package file.
	 * Can be called after name of the package has been set.
	 */
	QString
	file_name() const;

	/**
	 * Removes file associated with the package.
	 */
	void
	remove_file();

	/**
	 * Loads package data from given file.
	 */
	void
	load_file (QString const& path);

	/**
	 * Saves package to given file. Overwrites existing file.
	 */
	void
	save_file (QString const& path);

	/**
	 * Saves package to currently used file.
	 */
	void
	save_file();

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
	QString		_loaded_file_name;
	QString		_unit_urn;
	QString		_name;
	QString		_version;
	QString		_created_at;
	QString		_credits;
	QString		_license;
	Categories	_categories;
};

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

