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
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/id_allocator.h>

// Local:
#include "has_widget.h"
#include "part.h"
#include "part_widget.h"


namespace Yuki {

class Plugin;

class PartManager: public HasWidget<PartManagerWidget>
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
	 * Add new Part.
	 * Add UI widget.
	 */
	void
	add_part();

	/**
	 * Remove given Part.
	 * Remove UI widget.
	 */
	void
	remove_part (Part*);

	/**
	 * Remove all Parts.
	 * Remove UI widgets.
	 */
	void
	remove_all_parts();

	/**
	 * Add first Part (and tab widget) if there is none.
	 */
	void
	ensure_there_is_at_least_one_part();

	/**
	 * Moves part position in a list of parts.
	 * Position counts from 0.
	 */
	void
	set_part_position (Part*, unsigned int position);

  private:
	Plugin*				_plugin;
	Parts				_parts;
	Mutex				_parts_mutex;
	IDAllocator			_id_alloc;
};

} // namespace Yuki

#endif

