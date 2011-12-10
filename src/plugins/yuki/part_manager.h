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
#include <haruhi/graph/event_buffer.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/id_allocator.h>
#include <haruhi/graph/audio_port.h>
#include <haruhi/graph/event_port.h>

// Local:
#include "has_widget.h"
#include "has_plugin.h"
#include "params.h"
#include "part.h"
#include "part_widget.h"


namespace Yuki {

class Plugin;

class PartManager:
	public HasWidget<PartManagerWidget>,
	public HasPlugin
{
  public:
	PartManager (Plugin*);

	~PartManager();

	/**
	 * Return reference to set of Parts used by synth.
	 */
	Parts&
	parts();

	Parts const&
	parts() const;

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

	/**
	 * Process events.
	 */
	void
	process();

	/**
	 * Panic all voices.
	 */
	void
	panic();

	/**
	 * Resize buffers, etc.
	 */
	void
	graph_updated();

  private:
	Params::MainControls	_params;
	Parts					_parts;
	Mutex					_parts_mutex;
	IDAllocator				_id_alloc;

	// Ports (0 = L, 1 = R):
	Haruhi::AudioPort*		_audio_out[2];
	Haruhi::EventPort*		_voice_in;
};


inline Parts&
PartManager::parts()
{
	return _parts;
}


inline Parts const&
PartManager::parts() const
{
	return _parts;
}

} // namespace Yuki

#endif

