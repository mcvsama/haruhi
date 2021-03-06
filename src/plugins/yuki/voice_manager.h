/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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
#include <haruhi/dsp/filter.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/graph/event.h>
#include <haruhi/utility/work_performer.h>
#include <haruhi/utility/pool_allocator.h>

// Local:
#include "voice.h"
#include "params.h"


namespace Yuki {

using Haruhi::Unique;


/**
 * Creates/destroys/mixes Voices upon incoming Core Events.
 */
class VoiceManager
{
	class RenderWorkUnit;

	typedef std::set<Unique<Voice>> Voices;
	typedef DSP::Filter<FilterImpulseResponse::Order, FilterImpulseResponse::ResponseType> AntialiasingFilter;
	typedef std::map<Haruhi::VoiceID, Voices::iterator> ID2VoiceMap;
	typedef std::vector<Unique<RenderWorkUnit>> WorkUnits;
	typedef std::vector<Unique<Voice::SharedResources>> SharedResourcesVec;

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
	 * Process incoming VoiceEvent.
	 */
	void
	handle_voice_event (Haruhi::VoiceEvent const*);

	/**
	 * Process incoming event from the voice amplitude (velocity) port.
	 */
	void
	handle_amplitude_event (Haruhi::VoiceControllerEvent const*);

	/**
	 * Process incoming event from voice frequency (pitch) port.
	 */
	void
	handle_frequency_event (Haruhi::VoiceControllerEvent const*);

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
	graph_updated (Frequency sample_rate, std::size_t buffer_size);

	/**
	 * Set oversampling factor.
	 * Needs Graph lock.
	 */
	void
	set_oversampling (unsigned int oversampling);

	/**
	 * Make all current and future voices use given Wave.
	 */
	void
	set_wave (DSP::Wave*);

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
	async_render();

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
	 * Update buffers sizes according to Graph params and oversampling.
	 */
	void
	resize_buffers();

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
	WorkPerformer*			_work_performer;
	Params::Main*			_main_params;
	Params::Part*			_part_params;
	Voices					_voices;
	WorkUnits				_work_units;
	ID2VoiceMap				_voices_by_id;
	SharedResourcesVec		_shared_resources_vec;
	Frequency				_sample_rate			= 0_Hz;
	std::size_t				_buffer_size			= 0;
	unsigned int			_oversampling			= 1;
	FilterImpulseResponse	_antialiasing_filter_ir;
	AntialiasingFilter		_antialiasing_filter_1[5]; // Multi-stage filtering, number must be odd.
	AntialiasingFilter		_antialiasing_filter_2[5]; // Multi-stage filtering, number must be odd.
	DSP::Wave*				_wave					= nullptr;
	Haruhi::AudioBuffer		_output_1;
	Haruhi::AudioBuffer		_output_2;
	Haruhi::AudioBuffer		_output_1_oversampled;
	Haruhi::AudioBuffer		_output_2_oversampled;
	Haruhi::AudioBuffer		_output_1_filtered;
	Haruhi::AudioBuffer		_output_2_filtered;
	unsigned int			_active_voices_number	= 0;
	Frequency				_last_voice_frequency	= 440_Hz; // Concert A
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
			for (auto& v: _voices)
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

