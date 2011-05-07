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

#ifndef HARUHI__PLUGIN__PLUGIN_FACTORY_H__INCLUDED
#define HARUHI__PLUGIN__PLUGIN_FACTORY_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>


namespace Haruhi {

class Plugin;

class PluginFactory
{
  public:
	enum Type
	{
		Synthesizer	= 0,
		Effect		= 1,
		Other		= 2,
	};

  public:
	PluginFactory();

	virtual ~PluginFactory() { }

	/**
	 * Creates new plugin and returns pointer to it.
	 * Created plugin must be destroyed with destroy_plugin().
	 */
	virtual Plugin*
	create_plugin (int id, QWidget* parent) = 0;

	/**
	 * Destroys given plugin.
	 */
	virtual void
	destroy_plugin (Plugin*) = 0;

	/**
	 * Returns plugin URN.
	 */
	virtual const char*
	urn() const = 0;

	/**
	 * Returns plugin name/title.
	 */
	virtual const char*
	title() const = 0;

	/**
	 * Returns type of the plugin.
	 */
	virtual Type
	type() const = 0;

	/**
	 * Returns plugin author name. May be name of an individual,
	 * nick or company name.
	 */
	virtual const char*
	author() const = 0;

	/**
	 * Returns plugin author contact URIs. Should be standard URIs like:
	 * - xmpp:jabberid@example.org
	 * - mailto:email@example.org
	 * - http://authors-website.example.org
	 * Array must end with 0 value. May return 0.
	 */
	virtual const char**
	author_contacts() const = 0;

	/**
	 * License contents.
	 */
	virtual const char*
	license() const = 0;
};

} // namespace Haruhi

#endif

