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

#ifndef HARUHI__PLUGINS__YUKI__VOICE_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__VOICE_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/event.h>
#include <haruhi/dsp/ramp_smoother.h>

// Local:
#include "params.h"
#include "voice_oscillator.h"


namespace Yuki {

class Voice;

typedef std::set<Voice*> Voices;

class Voice
{
  public:
	enum State { Voicing, Dropped, Finished };

	/**
	 * Shared buffers for each thread of the RT work performer.
	 * Used by Voices that are being synthesized.
	 */
	struct SharedResources
	{
		void
		graph_updated (unsigned int sample_rate, std::size_t buffer_size);

		Haruhi::AudioBuffer	amplitude_buf;
		Haruhi::AudioBuffer	frequency_buf;
		Haruhi::AudioBuffer	tmp_buf;
	};

  public:
	/**
	 * \param	voice_params Voice params template.
	 */
	Voice (Haruhi::VoiceID id, Haruhi::Timestamp timestamp, Params::Main* main_params, Params::Part* part_params, Params::Voice* voice_params,
		   Sample amplitude, Sample frequency, unsigned int sample_rate, std::size_t buffer_size);

	/**
	 * Return voice's ID which came in Haruhi::VoiceEvent.
	 */
	Haruhi::VoiceID
	id() const;

	/**
	 * Return voice's timestamp.
	 */
	Haruhi::Timestamp
	timestamp() const;

	/**
	 * Return current voice state.
	 */
	State
	state() const;

	/**
	 * Drop voice. Voice does not immediately stop sounding.
	 * Use finished() to check if voice generation is really finished.
	 */
	void
	drop();

	/**
	 * Render voice.
	 * \return	true if something were actually synthesized, false otherwise.
	 */
	bool
	render (SharedResources*);

	/**
	 * Mix rendered voice into given buffers.
	 */
	void
	mix_result (Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2) const;

	/**
	 * Update buffers sizes.
	 */
	void
	graph_updated (unsigned int sample_rate, std::size_t buffer_size);

	/**
	 * Make voice use given Wavetable.
	 */
	void
	set_wavetable (DSP::Wavetable*);

	/**
	 * RW accessor to Voice params.
	 */
	Params::Voice*
	params();

	/**
	 * Set voice amplitude.
	 * \param	amplitude Amplitude value [0..1]
	 */
	void
	set_amplitude (Sample amplitude);

	/**
	 * Set new target absolute frequency of the voice.
	 * \param	frequency Absolute frequency [0..0.5]
	 */
	void
	set_frequency (Sample frequency);

  public:
	/**
	 * Return older from the two voices by comparing
	 * their timestamps.
	 */
	static Voice*
	return_older (Voice* a, Voice* b);

  private:
	/**
	 * Update glide/portamento params when
	 * voice frequency changes.
	 */
	void
	update_glide_parameters();

	/**
	 * Prepare amplitude buffer as amplitude source for VoiceOscillator.
	 * \param	amplitude_buf Buffer where result is stored.
	 */
	void
	prepare_amplitude_buffer (Haruhi::AudioBuffer* amplitude_buf);

	/**
	 * Prepare frequency buffer as frequency source for VoiceOscillator.
	 * \param	frequency_buf Buffer where result is stored.
	 * \param	tmp_buf Helper buffer.
	 */
	void
	prepare_frequency_buffer (Haruhi::AudioBuffer* frequency_buf, Haruhi::AudioBuffer* tmp_buf);

  private:
	Haruhi::VoiceID		_id;
	Haruhi::Timestamp	_timestamp;
	State				_state;
	Params::Voice		_params;
	Params::Part*		_part_params;
	Params::Main*		_main_params;
	Sample				_amplitude;
	Sample				_frequency;
	unsigned int		_sample_rate;
	std::size_t			_buffer_size;
	VoiceOscillator		_vosc;
	Haruhi::AudioBuffer	_output_1;
	Haruhi::AudioBuffer	_output_2;

	// Smoothers:
	DSP::RampSmoother	_smoother_amplitude;
	DSP::RampSmoother	_smoother_frequency;
	DSP::RampSmoother	_smoother_pitchbend;
	DSP::RampSmoother	_smoother_panorama_1;
	DSP::RampSmoother	_smoother_panorama_2;

	// Glide params:
	Sample				_target_frequency;
	Sample				_frequency_change;

	// Pitchbend params:
	float				_last_pitchbend_value;

	// Attack/drop: when voice starts to sound, it does not start immediately but instead
	// in a time period defined by _attack_samples. Similarly with dropping.
	std::size_t			_attack_sample;
	std::size_t			_attack_samples;
	std::size_t			_drop_sample;
	std::size_t			_drop_samples;

	// Set initially to true, reset after first mixin():
	bool				_first_pass;
};


inline Haruhi::VoiceID
Voice::id() const
{
	return _id;
}


inline Haruhi::Timestamp
Voice::timestamp() const
{
	return _timestamp;
}


inline Voice::State
Voice::state() const
{
	return _state;
}


inline void
Voice::drop()
{
	_state = Dropped;
}


inline void
Voice::mix_result (Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2) const
{
	output_1->mixin (&_output_1);
	output_2->mixin (&_output_2);
}


inline Params::Voice*
Voice::params()
{
	return &_params;
}


inline void
Voice::set_wavetable (DSP::Wavetable* wavetable)
{
	_vosc.set_wavetable (wavetable);
}


inline void
Voice::set_amplitude (Sample amplitude)
{
	_amplitude = amplitude;
}


inline void
Voice::set_frequency (Sample frequency)
{
	_target_frequency = frequency;
	update_glide_parameters();
}


inline Voice*
Voice::return_older (Voice* a, Voice* b)
{
	if (a->timestamp() < b->timestamp())
		return a;
	return b;
}

} // namespace Yuki

#endif

