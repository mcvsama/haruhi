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

#ifndef HARUHI__PLUGINS__YUKI__VOICE_MANAGER_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__VOICE_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/graph/event.h>

// Local:
#include "has_plugin.h"
#include "voice.h"


namespace Yuki {

/**
 * Creates/destroys/mixes Voices upon incoming Core Events.
 */
class VoiceManager: public HasPlugin
{
	typedef std::map<Haruhi::VoiceID, Voices::iterator> ID2VoiceMap;

  public:
	VoiceManager (Plugin* plugin);

	~VoiceManager();

	/**
	 * Gets maximum polyphony.
	 */
	unsigned int
	max_polyphony() const { return _max_polyphony; }

	/**
	 * Sets maximum polyphony.
	 */
	void
	set_max_polyphony (unsigned int num) { _max_polyphony = num; }

	/**
	 * Processes new VoiceEvent.
	 */
	void
	handle_voice_event (Haruhi::VoiceEvent const*);

	/**
	 * Drop all existing voices now.
	 */
	void
	panic();

	/**
	 * Return number of sounding voices.
	 */
	unsigned int
	current_voices_number() { return _active_voices_number; }

	/**
	 * Forward 'graph_updated' message to all voices.
	 */
	void
	graph_updated();

	/**
	 * Mix all voices into given buffers.
	 * This operation is done in separate thread(s),
	 * using prioritized WorkPerformer from Haruhi's
	 * Session object.
	 *
	 * This function blocks until data is ready.
	 * Buffers are not cleared before operation.
	 */
	void
	render (Haruhi::AudioBuffer*, Haruhi::AudioBuffer*);

  private:
	/**
	 * Check polyphony limit and drop excess voices.
	 */
	void
	check_polyphony_limit();

	/**
	 * Return voice by its ID. Return 0 if not found.
	 */
	Voice*
	find_voice_by_id (Haruhi::VoiceID);

	/**
	 * Kills all voices - deletes them immediately.
	 */
	void
	kill_voices();

  private:
	Voices				_voices;
	ID2VoiceMap			_voices_by_id;
	Haruhi::AudioBuffer	_tmpbuf1;
	Haruhi::AudioBuffer	_tmpbuf2;
	unsigned int		_active_voices_number;
	unsigned int		_max_polyphony;
};

} // namespace Yuki

#endif

