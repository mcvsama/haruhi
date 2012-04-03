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

#ifndef HARUHI__PLUGINS__EG__EG_H__INCLUDED
#define HARUHI__PLUGINS__EG__EG_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/plugin/plugin.h>
#include <haruhi/plugin/plugin_factory.h>


class EGFactory: public Haruhi::PluginFactory
{
  public:
	Haruhi::Plugin*
	create_plugin (int id, QWidget* parent);

	void
	destroy_plugin (Haruhi::Plugin* plugin);

	const char*
	urn() const { return  "urn://haruhi.mulabs.org/synth/eg/1"; }

	const char*
	title() const { return "EG"; }

	Type
	type() const { return Envelope; }

	const char*
	author() const { return "Michał <mcv> Gawron"; }

	const char**
	author_contacts() const;

	const char*
	license() const { return "GPL-3.0"; }
};

#endif

