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

#ifndef HARUHI__PLUGINS__YUKI__HAS_PLUGIN_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__HAS_PLUGIN_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/graph/graph.h>

// Local:
#include "plugin.h"


namespace Yuki {

class HasPlugin
{
  public:
	HasPlugin (Plugin* plugin) noexcept;

	/**
	 * Return pointer to the Yuki Plugin.
	 */
	Plugin*
	plugin() const noexcept;

	/**
	 * Shortcut to plugin()->graph().
	 */
	Haruhi::Graph*
	graph() const noexcept;

  private:
	Plugin* _plugin;
};


inline
HasPlugin::HasPlugin (Plugin* plugin) noexcept:
	_plugin (plugin)
{ }


inline Plugin*
HasPlugin::plugin() const noexcept
{
	return _plugin;
}


inline Haruhi::Graph*
HasPlugin::graph() const noexcept
{
	return _plugin->graph();
}

} // namespace Yuki

#endif

