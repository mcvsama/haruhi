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
#include <plugins/freeverb/freeverb.h>

// Local:
#include "plugin_loader.h"


namespace Haruhi {

PluginLoader::PluginLoader()
{
	_plugin_factories.push_back (std::make_unique<BugFuzzerFactory>());
	_plugin_factories.push_back (std::make_unique<EGFactory>());
	_plugin_factories.push_back (std::make_unique<YukiFactory>());
	_plugin_factories.push_back (std::make_unique<FreeverbFactory>());
}


PluginLoader::~PluginLoader()
{ }


Plugin*
PluginLoader::load (std::string const& urn, int id)
{
	if (auto factory = find_factory (urn))
	{
		auto plugin = factory->create_plugin (id, nullptr);
		plugin->hide();
		return plugin;
	}
	else
		return nullptr;
}


void
PluginLoader::unload (Plugin* plugin)
{
	if (auto factory = find_factory (plugin->urn()))
		factory->destroy_plugin (plugin);
	else
		throw Exception ("couldn't find factory to unload plugin");
}


PluginLoader::PluginFactories const&
PluginLoader::plugin_factories() const
{
	return _plugin_factories;
}


PluginFactory*
PluginLoader::find_factory (std::string const& urn) const
{
	for (auto& pf: _plugin_factories)
		if (pf->urn() == urn)
			return pf.get();
	return nullptr;
}

} // namespace Haruhi

