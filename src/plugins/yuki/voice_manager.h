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
#include <haruhi/utility/pool_allocator.h>

// Local:
#include "voice.h"
#include "params.h"


namespace Yuki {

/**
 * Creates/destroys/mixes Voices upon incoming Core Events.
 */
class VoiceManager
{
	class RenderWorkUnit;

	typedef std::map<Haruhi::VoiceID, Voices::iterator> ID2VoiceMap;
	typedef std::vector<RenderWorkUnit*> WorkUnits;
	typedef std::vector<Voice::SharedResources*> SharedResourcesVec;

	class RenderWorkUnit: public WorkPerformer::Unit
	{
		USES_POOL_ALLOCATOR (RenderWorkUnit)

	  public:
		RenderWorkUnit (Voice* voice, SharedResourcesVec& resources_vec);

		void
		execute();

		void
		mix_result (Haruhi::AudioBuffer*, Haruhi::AudioBuffer*) const;

	  private:
		Voice*				_voice;
		SharedResourcesVec&	_resources_vec;
	};

  public:
	VoiceManager (Params::Main*, Params::Part*, WorkPerformer*);

	~VoiceManager();

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
	 * Make all current and future voices use given Wavetable.
	 */
	void
	set_wavetable (DSP::Wavetable*);

	/**
	 * Return number of sounding voices.
	 * Does not include voices that has been just dropped, although
	 * they will contribute to the mixed result for a short while
	 * (a couple of milliseconds).
	 */
	unsigned int
	current_voices_number();

	/**
	 * Start rendering of all voices.
	 *
	 * This function is non-blocking. It will create WorkUnits
	 * and pass them to configured WorkPerformer.
	 *
	 * Use wait_for_render() to wait until rendering is done.
	 * Use mix_rendering_result() to mix rendered voices into given output buffers.
	 */
	void
	render();

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
	mix_rendering_result (Haruhi::AudioBuffer*, Haruhi::AudioBuffer*) noexcept;

	/**
	 * Update particular parameter of a particular voice.
	 * \param	voice_id ID of the voice to be updated. Use Haruhi::OmniVoice to update all voices.
	 */
	void
	update_voice_parameter (Haruhi::VoiceID, Params::Voice::ControllerParamPtr, int value);

	/**
	 * Update particular parameter of a particular voice.
	 * Handles both ControllerParams and Params<T>.
	 * \param	voice_id ID of the voice to be updated. Use Haruhi::OmniVoice to update all voices.
	 * \param	filter_no Filter ID, 0 or 1.
	 */
	template<class PointerToParam>
		void
		update_filter_parameter (Haruhi::VoiceID, unsigned int filter_no, PointerToParam, int value);

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
	WorkPerformer*		_work_performer;
	Params::Main*		_main_params;
	Params::Part*		_part_params;
	Voices				_voices;
	WorkUnits			_work_units;
	ID2VoiceMap			_voices_by_id;
	SharedResourcesVec	_shared_resources_vec;
	unsigned int		_sample_rate;
	std::size_t			_buffer_size;
	DSP::Wavetable*		_wavetable;
	Haruhi::AudioBuffer	_output_1;
	Haruhi::AudioBuffer	_output_2;
	unsigned int		_active_voices_number;
};


inline unsigned int
VoiceManager::current_voices_number()
{
	return _active_voices_number;
}


template<class PointerToParam>
	inline void
	VoiceManager::update_filter_parameter (Haruhi::VoiceID voice_id, unsigned int filter_no, PointerToParam param_ptr, int value)
	{
		if (voice_id == Haruhi::OmniVoice)
		{
			for (Voice* v: _voices)
				(v->params()->filters[filter_no].*param_ptr).set (value);
		}
		else
		{
			Voice* v = find_voice_by_id (voice_id);
			if (v)
				(v->params()->filters[filter_no].*param_ptr).set (value);
		}
	}

} // namespace Yuki

#endif

