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

#ifndef HARUHI__PLUGINS__YUKI__VOICE_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__VOICE_H__INCLUDED

// Standard:
#include <cstddef>
#include <new>
#include <set>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/event.h>
#include <haruhi/dsp/ramp_smoother.h>
#include <haruhi/utility/amplitude.h>
#include <haruhi/utility/normalized_frequency.h>

// Local:
#include "params.h"
#include "voice_modulator.h"
#include "voice_oscillator.h"
#include "dual_filter.h"


namespace Yuki {

class alignas (std::hardware_destructive_interference_size)
Voice
{
  public:
	enum State { Voicing, Dropped, Finished };

	/**
	 * Shared buffers for each thread of the RT work performer.
	 * Used by Voices that are being synthesized.
	 */
	struct alignas (std::hardware_destructive_interference_size)
	SharedResources
	{
		void
		graph_updated (Frequency sample_rate, std::size_t buffer_size);

		/**
		 * Needs Graph lock.
		 */
		void
		set_oversampling (unsigned int oversampling);

		Haruhi::AudioBuffer	amplitude_buf;
		Haruhi::AudioBuffer	frequency_buf;
		Haruhi::AudioBuffer	fm_buf;
		Haruhi::AudioBuffer	tmp_buf[6];

	  private:
		void
		resize_buffers();

	  private:
		std::size_t			_buffer_size	= 1;
		unsigned int		_oversampling	= 1;
	};

  private:
	static constexpr Time	AttackTime		= 1_ms;
	static constexpr Time	DropTime		= 1_ms;

  public:
	// Ctor.
	Voice (Haruhi::VoiceID id, Time timestamp, Params::Main* main_params, Params::Part* part_params,
		   Amplitude amplitude, NormalizedFrequency frequency, Frequency sample_rate, std::size_t buffer_size, unsigned int oversampling);

	/**
	 * Return voice's ID which came in Haruhi::VoiceEvent.
	 */
	Haruhi::VoiceID
	id() const noexcept;

	/**
	 * Return voice's timestamp.
	 */
	Time
	timestamp() const noexcept;

	/**
	 * Return current voice state.
	 */
	State
	state() const noexcept;

	/**
	 * Drop voice. Voice does not immediately stop sounding.
	 * Use finished() to check if voice generation is really finished.
	 */
	void
	drop() noexcept;

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
	mix_result (Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2) const noexcept;

	/**
	 * Update buffers sizes.
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
	 * Make voice use given Wave.
	 */
	void
	set_wave (DSP::Wave*);

	/**
	 * RW accessor to Voice params.
	 */
	Params::Voice*
	params() noexcept;

	/**
	 * Set voice amplitude.
	 * \param	amplitude Amplitude value [0..1]
	 */
	void
	set_amplitude (Amplitude amplitude) noexcept;

	/**
	 * Set new target normalized frequency of the voice.
	 */
	void
	set_frequency (Frequency frequency) noexcept;

  public:
	/**
	 * Return older from the two voices by comparing
	 * their timestamps.
	 */
	static Voice*
	return_older (Voice* a, Voice* b) noexcept;

  private:
	/**
	 * Update buffers sizes according to Graph params and oversampling.
	 */
	void
	resize_buffers();

	/**
	 * Precompute some handy variables and update smoothers.
	 */
	void
	recompute_sampling_rate_dependents() noexcept;

	/**
	 * Update glide/portamento params when
	 * voice frequency changes.
	 */
	void
	update_glide_parameters() noexcept;

	/**
	 * Prepare amplitude buffer as amplitude source for VoiceOscillator.
	 * \param	amplitude_buf Buffer where result is stored.
	 */
	void
	prepare_amplitude_buffer (Haruhi::AudioBuffer* amplitude_buf) noexcept;

	/**
	 * Prepare frequency buffer as frequency source for VoiceOscillator.
	 * \param	frequency_buf Buffer where result is stored.
	 * \param	tmp_buf Helper buffer.
	 */
	void
	prepare_frequency_buffer (Haruhi::AudioBuffer* frequency_buf, Haruhi::AudioBuffer* tmp_buf) noexcept;

  private:
	Haruhi::VoiceID		_id;
	Time				_timestamp;
	State				_state;
	Params::Voice		_params;
	Params::Part*		_part_params;
	Params::Main*		_main_params;
	Amplitude			_amplitude;
	NormalizedFrequency	_frequency;
	Frequency			_sample_rate;
	std::size_t			_buffer_size;
	unsigned int		_oversampling;
	VoiceModulator		_vmod;
	VoiceOscillator		_vosc;
	DualFilter			_dual_filter;
	Haruhi::AudioBuffer	_output_1;
	Haruhi::AudioBuffer	_output_2;

	// Smoothers:
	DSP::RampSmoother	_smoother_amplitude;
	DSP::RampSmoother	_smoother_frequency;
	DSP::RampSmoother	_smoother_pitchbend;
	DSP::RampSmoother	_smoother_panorama_1;
	DSP::RampSmoother	_smoother_panorama_2;

	// Glide params:
	NormalizedFrequency	_target_frequency;
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
Voice::id() const noexcept
{
	return _id;
}


inline Time
Voice::timestamp() const noexcept
{
	return _timestamp;
}


inline Voice::State
Voice::state() const noexcept
{
	return _state;
}


inline void
Voice::drop() noexcept
{
	_state = Dropped;
}


inline void
Voice::mix_result (Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2) const noexcept
{
	output_1->mixin (&_output_1);
	output_2->mixin (&_output_2);
}


inline Params::Voice*
Voice::params() noexcept
{
	return &_params;
}


inline void
Voice::set_wave (DSP::Wave* wave)
{
	_vosc.set_wave (wave);
}


inline void
Voice::set_amplitude (Amplitude amplitude) noexcept
{
	_amplitude = amplitude;
}


inline void
Voice::set_frequency (Frequency frequency) noexcept
{
	_target_frequency = frequency / _sample_rate;
	update_glide_parameters();
}


inline Voice*
Voice::return_older (Voice* a, Voice* b) noexcept
{
	if (a->timestamp() < b->timestamp())
		return a;
	return b;
}

} // namespace Yuki

#endif

