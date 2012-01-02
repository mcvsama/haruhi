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

#ifndef HARUHI__PLUGINS__YUKI__PLUGIN_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PLUGIN_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/session/unit_bay.h>
#include <haruhi/plugin/plugin.h>
#include <haruhi/plugin/has_presets.h>
#include <haruhi/utility/saveable_state.h>


namespace Yuki {

class PartManager;
class PartManagerWidget;

class Plugin:
	public Haruhi::Plugin,
	public Haruhi::UnitBayAware,
	public Haruhi::HasPresets,
	public SaveableState
{
	Q_OBJECT

  public:
	Plugin (std::string const& urn, std::string const& title, int id, QWidget* parent);

	virtual ~Plugin();

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

	int
	voices_number() const;

	void
	set_unit_bay (Haruhi::UnitBay*);

	/*
	 * SaveableState implementation
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

	/*
	 * HasPresets implementation.
	 */

	void
	save_preset (QDomElement& element) const;

	void
	load_preset (QDomElement const& element);

  private:
	PartManager*		_part_manager;
	PartManagerWidget*	_part_manager_widget;
};


inline void
Plugin::save_preset (QDomElement& element) const
{
	save_state (element);
}


inline void
Plugin::load_preset (QDomElement const& element)
{
	load_state (element);
}

} // namespace Yuki

#endif

