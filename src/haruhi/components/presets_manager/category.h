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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__CATEGORY_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__CATEGORY_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/saveable_state.h>

// Qt:
#include <QtCore/QString>

// Local:
#include "preset.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

class Category: public SaveableState
{
  public:
	typedef std::list<Preset> Presets;

  public:
	Category();

	/**
	 * Category name.
	 */
	QString
	name() const { return _name; }

	/**
	 * Sets category name.
	 */
	void
	set_name (QString const& name) { _name = name; }

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement& element) const;

	void
	load_state (QDomElement const& element);

  private:
	QString	_name;
	Presets	_presets;
};

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

