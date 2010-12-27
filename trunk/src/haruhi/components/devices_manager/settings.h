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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__SETTINGS_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__SETTINGS_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>

// Qt:
#include <QtCore/QString>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/settings/settings.h>

// Local:
#include "device.h"
#include "model.h"


namespace Haruhi {

namespace DevicesManager {

/**
 * Settings module for DevicesManager.
 * Contains Model.
 */
class Settings: public Haruhi::Settings::Module
{
  public:
	Settings();

	/**
	 * Model accessor.
	 */
	Model&
	model() { return _model; }

	/**
	 * Model accessor.
	 */
	Model const&
	model() const { return _model; }

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement& element) const;

	void
	load_state (QDomElement const& element);

  private:
	Model	_model;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

