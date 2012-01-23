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

#ifndef HARUHI__PLUGIN__PLUGIN_LOADER_H__INCLUDED
#define HARUHI__PLUGIN__PLUGIN_LOADER_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/unit.h>


namespace Haruhi {

class Plugin;
class PluginFactory;

/**
 * Manages units factories.
 */
class PluginLoader
{
  public:
	typedef std::list<PluginFactory*> PluginFactories;

  public:
	PluginLoader();

	~PluginLoader();

	Plugin*
	load (std::string const& urn, int id = 0);

	void
	unload (Plugin*);

	PluginFactories const&
	plugin_factories() const;

  private:
	PluginFactory*
	find_factory (std::string const& urn) const;

  private:
	PluginFactories	_plugin_factories;
};

} // namespace Haruhi

#endif

