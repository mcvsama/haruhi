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

// Standard:
#include <cstddef>
#include <algorithm>
#include <string>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/exception.h>
#include <haruhi/plugin/plugin.h>
#include <haruhi/plugin/plugin_factory.h>
#include <haruhi/utility/memory.h>

// Factories:
#include <units/mikuru/mikuru.h>
#include <units/vanhalen/vanhalen.h>

// Local:
#include "plugin_loader.h"


namespace Haruhi {

PluginLoader::PluginLoader (Session* session):
	_session (session)
{
	_plugin_factories.push_back (new MikuruFactory());
	_plugin_factories.push_back (new VanHalenFactory());
}


PluginLoader::~PluginLoader()
{
	std::for_each (_plugin_factories.begin(), _plugin_factories.end(), delete_operator<PluginFactories::value_type>);
}


Plugin*
PluginLoader::load (std::string const& urn, int id)
{
	PluginFactory* factory = find_factory (urn);
	if (factory)
	{
		Plugin* plugin = factory->create_plugin (_session, id, 0);
		plugin->hide();
		return plugin;
	}
	return 0;
}


void
PluginLoader::unload (Plugin* plugin)
{
	PluginFactory* factory = find_factory (plugin->urn());
	if (!factory)
		throw Exception ("couldn't find factory to unload plugin");
	factory->destroy_plugin (plugin);
}


PluginLoader::PluginFactories const&
PluginLoader::plugin_factories() const
{
	return _plugin_factories;
}


PluginFactory*
PluginLoader::find_factory (std::string const& urn) const
{
	for (PluginFactories::const_iterator f = _plugin_factories.begin();  f != _plugin_factories.end();  ++f)
	{
		PluginFactory::InformationMap const& map = (*f)->information();
		PluginFactory::InformationMap::const_iterator i = map.find ("haruhi:urn");
		if (i == map.end())
			continue;
		if (i->second == urn)
			return *f;
	}
	return 0;
}

} // namespace Haruhi

