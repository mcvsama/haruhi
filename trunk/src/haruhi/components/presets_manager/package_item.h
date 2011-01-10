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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__PACKAGE_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__PACKAGE_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/utility/saveable_state.h>

// Local:
#include "presets_manager.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

class CategoryItem;

class PackageItem: public QTreeWidgetItem
{
  public:
	class Meta: public SaveableState
	{
	  public:
		void
		save_state (QDomElement& meta) const;

		void
		load_state (QDomElement const& meta);

	  public:
		QString		name;
		QString		version;
		QString		created_at;
		QString		credits;
		QString		license;
	};

  public:
	PackageItem (PresetsTree* parent);

	void
	setup();

	PresetsManager*
	presets_manager() const { return _presets_manager; }

	void
	reload();

	Meta&
	meta() { return _meta; }

	Meta const&
	meta() const { return _meta; }

	void
	load_file (QString const& file_name);

	void
	save_file();

	void
	save_file (QString const& file_name);

	void
	remove_file();

	QDomDocument&
	document() { return _document; }

	/*
	 * SaveableState interface
	 */

	void
	save_state (QDomElement& meta) const;

	void
	load_state (QDomElement const& meta);

  private:
	CategoryItem*
	find_or_create_category (QString const& name);

  private:
	PresetsManager*	_presets_manager;
	QDomDocument	_document;
	QString			_file_name;
	QString			_unit_urn;
	Meta			_meta;
};

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

