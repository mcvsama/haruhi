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

// Standard:
#include <cstddef>
#include <algorithm>
#include <string>

// Haruhi:
#include <haruhi/application/haruhi.h>
#include <haruhi/plugin/plugin.h>
#include <haruhi/plugin/plugin_factory.h>
#include <haruhi/utility/memory.h>
#include <haruhi/utility/exception.h>

// Factories:
#include <plugins/bugfuzzer/bugfuzzer.h>
#include <plugins/eg/eg.h>
#include <plugins/yuki/yuki.h>

// Local:
#include "plugin_loader.h"


namespace Haruhi {

PluginLoader::PluginLoader()
{
	_plugin_factories.push_back (new BugFuzzerFactory());
	_plugin_factories.push_back (new EGFactory());
	_plugin_factories.push_back (new YukiFactory());
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
		Plugin* plugin = factory->create_plugin (id, 0);
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
	for (PluginFactory* pf: _plugin_factories)
		if (pf->urn() == urn)
			return pf;
	return 0;
}

} // namespace Haruhi

