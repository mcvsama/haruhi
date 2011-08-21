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

// Qt:

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "eg.h"


EG::EG (std::string const& urn, std::string const& title, int id, QWidget* parent):
	Plugin (urn, title, id, parent)
{
}


EG::~EG()
{
}


void
EG::registered()
{
}


void
EG::unregistered()
{
}


void
EG::process()
{
}


void
EG::panic()
{
}


void
EG::graph_updated()
{
}


void
EG::set_unit_bay (Haruhi::UnitBay*)
{
}


void
EG::save_state (QDomElement&) const
{
}


void
EG::load_state (QDomElement const&)
{
}


Haruhi::Plugin*
EGFactory::create_plugin (int id, QWidget* parent)
{
	return new EG (urn(), title(), id, parent);
}


void
EGFactory::destroy_plugin (Haruhi::Plugin* plugin)
{
	delete plugin;
}


const char**
EGFactory::author_contacts() const
{
	static const char* table[] = { "mailto:michal@gawron.name", "xmpp:mcv@jabber.org", 0 };
	return table;
}

