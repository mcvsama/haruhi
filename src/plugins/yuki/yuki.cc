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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "yuki.h"
#include "plugin.h"


Haruhi::Plugin*
YukiFactory::create_plugin (int id, QWidget* parent)
{
	return new Yuki::Plugin (urn(), title(), id, parent);
}


void
YukiFactory::destroy_plugin (Haruhi::Plugin* plugin)
{
	delete plugin;
}


const char**
YukiFactory::author_contacts() const
{
	static const char* tab[] = { "mailto:michal@gawron.name", "xmpp:mcv@jabber.org", 0 };
	return tab;
}

