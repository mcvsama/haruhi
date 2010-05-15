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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__PRESET_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__PRESET_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <Qt3Support/Q3ListView>

// Haruhi:
#include <haruhi/utility/saveable_state.h>

// Local:
#include "presets_manager.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

class CategoryItem;

class PresetItem: public Q3ListViewItem
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
	};

  public:
	PresetItem (CategoryItem* parent);

	CategoryItem*
	category_item() const;

	void
	reload();

	Meta&
	meta() { return _meta; }

	Meta const&
	meta() const { return _meta; }

	QDomElement&
	patch() { return _patch; }

	QDomElement const&
	patch() const { return _patch; }

	QString const&
	uuid() const { return _uuid; }

	void
	clear_patch_element (QDomDocument& document);

	void
	ensure_has_uuid();

	/*
	 * SaveableState interface
	 */

	void
	save_state (QDomElement& meta) const;

	void
	load_state (QDomElement const& meta);

  private:
	/**
	 * Generates and stores new UUID for this preset.
	 */
	void
	generate_uuid();

  private:
	QString		_uuid;
	Meta		_meta;
	QDomElement	_patch;
};

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

