/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__UNITS__MIKURU__VOICE_H__INCLUDED
#define HARUHI__UNITS__MIKURU__VOICE_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTabWidget>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>

// Haruhi:
#include <haruhi/graph/event.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/dsp/wavetable.h>
#include <haruhi/dsp/filter.h>
#include <haruhi/dsp/smoother.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "voice_oscillator.h"
#include "voice_commons.h"
#include "double_filter.h"
#include "params.h"


class Mikuru;

namespace MikuruPrivate {

class Part;
class SynthThread;
class VoiceManager;

using Haruhi::Sample;

class Voice
{
  public:
	class ReturnOlder
	{
	  public:
		Voice*
		operator() (Voice* a, Voice* b)
		{
			if (a->timestamp() < b->timestamp())
				return a;
			return b;
		}
	};

  public:
	/**
	 * Voice automatically assigns itself to specified thread.
	 */
	Voice (VoiceManager*, SynthThread*, Haruhi::KeyID key_id, Haruhi::VoiceID voice_id, Sample frequency, Sample amplitude, Haruhi::Timestamp timestamp);

	/**
	 * Automatically deassigns itself from specified thread, so thread will no longer
	 * try to process this Voice.
	 * \entry	Only processing thread.
	 */
	~Voice();

	Mikuru*
	mikuru() const { return _mikuru; }

	VoiceManager*
	voice_manager() const { return _voice_manager; }

	/**
	 * \returns	currently used SynthThread.
	 * \entry	Any thread.
	 */
	SynthThread*
	thread() const { return _synth_thread; }

	/**
	 * Moves Voice to another thread.
	 */
	void
	set_thread (SynthThread* thread);

	void
	set_amplitude (Sample amplitude) { _amplitude = amplitude; }

	void
	set_frequency (Sample frequency);

	/**
	 * Sets voice to tracked state, that means VoiceManager should not
	 * drop this voice upon Release event. Separate Drop event will be sent
	 * to drop this voice.
	 */
	void
	set_tracked (bool tracked) { _tracked = tracked; }

	bool
	tracked() const { return _tracked; }

	Params::Voice*
	params() { return &_params; }

	Params::Filter*
	filter1_params() { return &_filter1_params; }

	Params::Filter*
	filter2_params() { return &_filter2_params; }

	Haruhi::KeyID
	key_id() const { return _key_id; }

	Haruhi::VoiceID
	voice_id() const { return _voice_id; }

	Haruhi::Timestamp
	timestamp() const { return _timestamp; }

	/**
	 * Preconditions:
	 * distance (begin1, end1) = distance (begin2, end2)
	 *
	 * May be called only when Voice is assigned to existing thread.
	 */
	void
	mixin (Haruhi::AudioBuffer* output1, Haruhi::AudioBuffer* output2);

	void
	release();

	bool
	released() const { return _released; }

	void
	drop();

	/**
	 * Dropped voice does not stop sounding immediately.
	 * If you want to check if you can delete voice, use
	 * finished() instead.
	 */
	bool
	dropped() const { return _dropped; }

	/**
	 * Returns true if voice completely finished voicing
	 * and can be deleted.
	 */
	bool
	finished() const;

  private:
	inline void
	process_frequency();

	inline void
	process_amplitude();

	void
	update_glide_parameters();

  private:
	Mikuru*						_mikuru;
	Params::Voice				_params;
	Params::Filter				_filter1_params;
	Params::Filter				_filter2_params;

	Part*						_part;
	VoiceManager*				_voice_manager;
	SynthThread*				_synth_thread;
	VoiceCommons*				_commons;

	Haruhi::KeyID				_key_id;
	Haruhi::VoiceID				_voice_id;
	Haruhi::Timestamp			_timestamp;

	// Parameters:
	Sample						_frequency;
	Sample						_target_frequency;
	Sample						_frequency_change;
	Sample						_amplitude;

	// Components:
	VoiceOscillator				_oscillator;
	DoubleFilter				_double_filter;

	// Smoothers:
	DSP::Smoother				_smoother_amplitude;
	DSP::Smoother				_smoother_frequency;
	DSP::Smoother				_smoother_pitchbend;
	DSP::Smoother				_smoother_panorama_1;
	DSP::Smoother				_smoother_panorama_2;

	// Helpers, attack/release to prevent audio cracking, etc:
	float						_last_pitchbend_value;
	// When voice starts to sound, it does not start immediately but instead
	// in a time period defined by _attack_samples.
	unsigned int				_attack_sample;
	unsigned int				_attack_samples;
	bool						_released;
	// When voice is dropped, it's not silenced instantly but in a time
	// period defined by _drop_samples.
	unsigned int				_drop_sample;
	unsigned int				_drop_samples;
	bool						_dropped;
	bool						_tracked;
};


typedef std::set<Voice*> Voices;

} // namespace MikuruPrivate

#endif

