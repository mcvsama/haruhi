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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__PRESET_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__PRESET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QString>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/saveable_state.h>


namespace Haruhi {

namespace PresetsManagerPrivate {

class Preset: public SaveableState
{
  public:
	Preset();

	/**
	 * Preset name.
	 */
	QString
	name() const { return _name; }

	/**
	 * Sets preset name.
	 */
	void
	set_name (QString const& name) { _name = name; }

	/**
	 * Creates new XML node for patch.
	 */
	void
	clear_patch_element (QDomDocument& document);

	/**
	 * Patch XML element accessor.
	 */
	QDomElement&
	patch() { return _patch; }

	/**
	 * Patch XML element accessor.
	 */
	QDomElement const&
	patch() const { return _patch; }

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement& element) const;

	void
	load_state (QDomElement const& element);

  private:
	void
	generate_uuid();

  private:
	QString		_uuid;
	QString		_name;
	QString		_version;
	QString		_created_at;
	QDomElement	_patch;
};

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

