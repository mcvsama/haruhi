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

#ifndef HARUHI__PLUGINS__BUGFUZZER__BUGFUZZER_H__INCLUDED
#define HARUHI__PLUGINS__BUGFUZZER__BUGFUZZER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>

// Local:
#include <haruhi/config/all.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/plugin/plugin.h>
#include <haruhi/plugin/plugin_factory.h>


class BugFuzzer: public Haruhi::Plugin
{
	Q_OBJECT

	enum {
		EventPortsNum = 5,
	};

  public:
	BugFuzzer (std::string const& urn, std::string const& title, int id, QWidget* parent);

	virtual ~BugFuzzer();

	/**
	 * Unit API
	 */
	void
	registered();

	/**
	 * Unit API
	 */
	void
	unregistered();

	void
	process();

  private slots:
	void
	connect_ports();

  private:
	Haruhi::Event*
	get_random_event();

  private:
	Haruhi::EventPort*		_event_ports[EventPortsNum];
};


class BugFuzzerFactory: public Haruhi::PluginFactory
{
  public:
	BugFuzzerFactory();

	Haruhi::Plugin*
	create_plugin (int id, QWidget* parent);

	void
	destroy_plugin (Haruhi::Plugin* plugin);

	const char*
	urn() const { return  "urn://haruhi.mulabs.org/synth/bugfuzzer/1"; }

	const char*
	title() const { return "BugFuzzer"; }

	Type
	type() const { return Other; }

	const char*
	author() const { return "Michał <mcv> Gawron"; }

	const char**
	author_contacts() const { return 0; }

	const char*
	license() const { return "GPL-3.0"; }
};

#endif

