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
	name() const;

	void
	set_name (QString const& name);

	/**
	 * Package meta-information: version.
	 */
	QString
	version() const;

	void
	set_version (QString const& version);

	/**
	 * Package meta-information: created_at.
	 */
	QString
	created_at() const;

	void
	set_created_at (QString const& created_at);

	/**
	 * Package meta-information: created_at.
	 */
	QString
	credits() const;

	void
	set_credits (QString const& credits);

	/**
	 * Package meta-information: created_at.
	 */
	QString
	license() const;

	void
	set_license (QString const& license);

	/**
	 * Categories accessor.
	 */
	Categories&
	categories();

	/**
	 * Categories accessor.
	 */
	Categories const&
	categories() const;

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
	file_name() const;

	void
	set_file_name (QString const& file_name);

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement& element) const override;

	void
	load_state (QDomElement const& element) override;

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


inline QString
Package::name() const
{
	return _name;
}


inline void
Package::set_name (QString const& name)
{
	_name = name;
}


inline QString
Package::version() const
{
	return _version;
}


inline void
Package::set_version (QString const& version)
{
	_version = version;
}


inline QString
Package::created_at() const
{
	return _created_at;
}


inline void
Package::set_created_at (QString const& created_at)
{
	_created_at = created_at;
}


inline QString
Package::credits() const
{
	return _credits;
}


inline void
Package::set_credits (QString const& credits)
{
	_credits = credits;
}


inline QString
Package::license() const
{
	return _license;
}


inline void
Package::set_license (QString const& license)
{
	_license = license;
}


inline Package::Categories&
Package::categories()
{
	return _categories;
}


inline Package::Categories const&
Package::categories() const
{
	return _categories;
}


inline QString
Package::file_name() const
{
	return _file_name;
}


inline void
Package::set_file_name (QString const& file_name)
{
	_file_name = file_name;
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

