/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__SETTINGS__PLUGIN_SETTINGS_H__INCLUDED
#define HARUHI__SETTINGS__PLUGIN_SETTINGS_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "settings.h"


namespace Haruhi {

class PluginSettings: public Settings::Module
{
  public:
	PluginSettings();

	void
	save_state (QDomElement& element) const override;

	void
	load_state (QDomElement const& element) override;
};

} // namespace Haruhi

#endif

