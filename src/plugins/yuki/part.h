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

#ifndef HARUHI__PLUGINS__YUKI__PART_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/event.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/utility/work_performer.h>

// Local:
#include "has_widget.h"
#include "has_id.h"
#include "params.h"


namespace Yuki {

class PartWidget;
class PartManager;
class WaveComputer;
class VoiceManager;

class Part:
	public HasWidget<PartWidget>,
	public HasID
{
  public:
	Part (PartManager* part_manager);

	~Part();

	/**
	 * Return WaveComputer for this part.
	 * Uses plugin's global WaveComputer.
	 */
	WaveComputer*
	wave_computer() const;

	/**
	 * Handle voice input event.
	 * Pass it to the voice manager if conditions are met.
	 */
	void
	handle_voice_event (Haruhi::VoiceEvent const*);

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

	/**
	 * Start voices rendering.
	 * Use given WorkPerformer object.
	 */
	void
	render (WorkPerformer*);

	/**
	 * Wait until voice rendering is done.
	 */
	void
	wait_for_render();

	/**
	 * Mix rendered voices into given buffers.
	 */
	void
	mix_rendering_result (Haruhi::AudioBuffer*, Haruhi::AudioBuffer*);

  private:
	Params::Part	_params;
	PartManager*	_part_manager;
	VoiceManager*	_voice_manager;
};

typedef std::list<Part*>  Parts;

} // namespace Yuki

#endif

