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
#include <map>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/graph/event.h>
#include <haruhi/utility/work_performer.h>

// Local:
#include "voice.h"


namespace Yuki {

/**
 * Creates/destroys/mixes Voices upon incoming Core Events.
 */
class VoiceManager
{
	typedef std::map<Haruhi::VoiceID, Voices::iterator> ID2VoiceMap;
	typedef std::vector<WorkPerformer::Unit*> WorkUnits;

  public:
	VoiceManager();

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
	 * Update internal buffers params and forward
	 * 'graph_updated' message to all voices.
	 */
	void
	graph_updated (unsigned int sample_rate, std::size_t buffer_size);

	/**
	 * Return number of sounding voices.
	 * Does not include voices that has been just dropped, although
	 * they will contribute to the mixed result for a short while
	 * (a couple of milliseconds).
	 */
	unsigned int
	current_voices_number() { return _active_voices_number; }

	/**
	 * Start rendering of all voices.
	 *
	 * This function is non-blocking. It will create a WorkUnit
	 * and pass it to given WorkPerformer.
	 *
	 * Use wait_for_render() to wait until rendering is done.
	 * Use mix_result() to mix rendered voices into given output buffers.
	 */
	void
	render (WorkPerformer*);

	/**
	 * Block until rendering is done.
	 */
	void
	wait_for_render();

	/**
	 * Mix rendered voices into output buffer.
	 * Remove voices that are finished.
	 *
	 * Call render() once and wait with wait_for_render() in each
	 * processing round before mixing result.
	 */
	void
	mix_rendering_result (Haruhi::AudioBuffer*, Haruhi::AudioBuffer*);

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

	/**
	 * Render given voice into temporary buffers and add result to the output buffers.
	 */
	static void
	render_voice (Voice* voice,
				  Haruhi::AudioBuffer* tmp1, Haruhi::AudioBuffer* tmp2,
				  Haruhi::AudioBuffer* out1, Haruhi::AudioBuffer* out2);

  private:
	Voices				_voices;
	WorkUnits			_work_units;
	ID2VoiceMap			_voices_by_id;
	Haruhi::AudioBuffer	_tmp_voice_buf1;
	Haruhi::AudioBuffer	_tmp_voice_buf2;
	Haruhi::AudioBuffer	_tmp_mixed_buf1;
	Haruhi::AudioBuffer	_tmp_mixed_buf2;
	unsigned int		_active_voices_number;
	unsigned int		_max_polyphony;
};

} // namespace Yuki

#endif

