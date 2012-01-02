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
#include <map>
#include <set>

// Qt:
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/event.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/plugin/plugin.h>
#include <haruhi/plugin/plugin_factory.h>
#include <haruhi/session/unit_bay.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/id_allocator.h>

// Local:
#include "eg.h"


class EG:
	public Haruhi::Plugin,
	public Haruhi::UnitBayAware,
	public SaveableState
{
	Q_OBJECT

  public:
	EG (std::string const& urn, std::string const& title, int id, QWidget* parent);

	virtual ~EG();

	/*
	 * Plugin implementation.
	 */

	void
	registered();

	void
	unregistered();

	void
	process();

	void
	panic();

	void
	graph_updated();

	/*
	 * Haruhi::HasUnitBay implementation.
	 */

	void
	set_unit_bay (Haruhi::UnitBay*);

	/*
	 * SaveableState implementation.
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  private:
	QPushButton*					_muted;
	IDAllocator						_adsr_ids;
	IDAllocator						_lfo_ids;
	IDAllocator						_eg_ids;

	Haruhi::EventPort*				_port_keyboard;
	Haruhi::EventPort*				_port_sustain;
};


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

