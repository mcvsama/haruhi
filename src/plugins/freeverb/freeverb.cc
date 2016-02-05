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

// Local:
#include "freeverb.h"
#include "plugin.h"


Haruhi::Plugin*
FreeverbFactory::create_plugin (int id, QWidget* parent)
{
	return new Freeverb::Plugin (urn(), title(), id, parent);
}


void
FreeverbFactory::destroy_plugin (Haruhi::Plugin* plugin)
{
	delete plugin;
}


const char**
FreeverbFactory::author_contacts() const
{
	static const char* tab[] = { "mailto:michal@gawron.name", 0 };
	return tab;
}

