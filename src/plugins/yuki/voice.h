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
	enum State { NotStarted, Attacking, Voicing, Dropped, Finished };

	/**
	 * Shared buffers for each thread of the RT work performer.
	 * Used by Voices that are being synthesized.
	 */
	struct SharedResources
	{
		void
		graph_updated (unsigned int sample_rate, std::size_t buffer_size);

		Haruhi::AudioBuffer amplitude_buf;
		Haruhi::AudioBuffer frequency_buf;
	};

  public:
	Voice (Haruhi::VoiceID id, Haruhi::Timestamp timestamp, Params::Part* part_params, Sample amplitude, Sample frequency, unsigned int sample_rate, std::size_t buffer_size);

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

  public:
	/**
	 * Return older from the two voices by comparing
	 * their timestamps.
	 */
	static Voice*
	return_older (Voice* a, Voice* b);

  private:
	Haruhi::VoiceID		_id;
	Haruhi::Timestamp	_timestamp;
	State				_state;
	Params::Voice		_params;
	Params::Part*		_part_params;
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
	_state = Finished;
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


inline Voice*
Voice::return_older (Voice* a, Voice* b)
{
	if (a->timestamp() < b->timestamp())
		return a;
	return b;
}

} // namespace Yuki

#endif

