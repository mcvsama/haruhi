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
#include "eg.h"
#include "plugin.h"


Haruhi::Plugin*
EGFactory::create_plugin (int id, QWidget* parent)
{
	return new EG::Plugin (urn(), title(), id, parent);
}


void
EGFactory::destroy_plugin (Haruhi::Plugin* plugin)
{
	delete plugin;
}


const char**
EGFactory::author_contacts() const
{
	static const char* table[] = { "mailto:michal@gawron.name", 0 };
	return table;
}

