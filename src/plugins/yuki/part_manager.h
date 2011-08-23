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

#ifndef HARUHI__PLUGINS__YUKI__PART_MANAGER_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTabWidget>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "part.h"
#include "part_widget.h"


namespace Yuki {

class Plugin;

class PartManager
{
  public:
	PartManager (Plugin*);

	/**
	 * Return reference to set of Parts used by synth.
	 */
	Parts&
	parts() { return _parts; }

	Parts const&
	parts() const { return _parts; }

	/**
	 * Return Plugin instance that owns this PartManager.
	 */
	Plugin*
	plugin() const { return _plugin; }

	/**
	 * Return tab-position of given part.
	 */
	int
	part_tab_position (Part* part) const { return _tabs->indexOf (part->widget()); }

  public slots:
	/**
	 * Add first Part (and tab widget) if there are none.
	 */
	void
	ensure_there_is_at_least_one_part();

	/**
	 * Create new part (oscillator, filters, etc).
	 */
	Part*
	add_part();

	/**
	 * Destroy currently selected (as a tab) part.
	 * Remove tab widget and associated Part object.
	 */
	void
	del_part();

	/**
	 * Destroy part by its pointer.
	 * Remove tab widget.
	 */
	void
	del_part (Part*);

	/**
	 * Remove all tabs and Parts.
	 */
	void
	del_all_parts();

  private:
	Plugin*		_plugin;
	Parts		_parts;
	QTabWidget*	_tabs;
};

} // namespace Yuki

#endif

