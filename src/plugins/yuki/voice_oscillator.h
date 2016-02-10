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

#ifndef HARUHI__PLUGINS__YUKI__VOICE_OSCILLATAOR_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__VOICE_OSCILLATAOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/dsp/noise.h>
#include <haruhi/dsp/functions.h>
#include <haruhi/utility/amplitude.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/normalized_frequency.h>


namespace Yuki {

namespace DSP = Haruhi::DSP;
using Haruhi::Sample;

class VoiceOscillator
{
	enum {
		MaxUnison = 10,
	};

  public:
	/**
	 * If oscillator has no assigned wave, it will
	 * mixin/fill silence.
	 */
	VoiceOscillator (DSP::Wave* wave = nullptr) noexcept;

	/**
	 * Enable/disable wave generator.
	 */
	void
	set_wave_enabled (bool enabled) noexcept;

	/**
	 * Set new wave. May be 0.
	 */
	void
	set_wave (DSP::Wave* wave) noexcept;

	/**
	 * Return currently used wave.
	 */
	DSP::Wave*
	wave() const noexcept;

	/**
	 * Set frequency source buffer.
	 */
	void
	set_frequency_source (Haruhi::AudioBuffer* source) noexcept;

	/**
	 * Set amplitude source buffer.
	 */
	void
	set_amplitude_source (Haruhi::AudioBuffer* source) noexcept;

	/**
	 * Set FM modulator source.
	 * Changing frequency in this buffer doesn't cause wave jumping.
	 */
	void
	set_fm_source (Haruhi::AudioBuffer* source) noexcept;

	/**
	 * Enable/disable noise generator.
	 */
	void
	set_noise_enabled (bool enabled) noexcept;

	/**
	 * Set noise amplitude.
	 */
	void
	set_noise_amplitude (Amplitude amplitude) noexcept;

	/**
	 * Argument: [-1.0…1.0]
	 */
	void
	set_phase (Sample phase) noexcept;

	/**
	 * Argument: [-1.0…1.0]
	 */
	void
	set_initial_phases_spread (Sample spread) noexcept;

	/**
	 * Argument: [1…MaxUnison]
	 */
	void
	set_unison_number (int number) noexcept;

	/**
	 * Argument: [0…1.0]
	 */
	void
	set_unison_spread (Sample spread) noexcept;

	/**
	 * Argument: [0…1.0] (0.0 disables noise completely).
	 */
	void
	set_unison_noise (Sample noise) noexcept;

	/**
	 * Enable/disable stereo spread of unison voices.
	 */
	void
	set_unison_stereo (bool stereo) noexcept;

	/**
	 * Set unison voices vibrato level.
	 * \param	level Input param [0.0..1.0].
	 */
	void
	set_unison_vibrato_level (Sample level) noexcept;

	/**
	 * Set unison voices vibrato frequency.
	 * \param	frequency Normalized frequency [0.0..0.5].
	 */
	void
	set_unison_vibrato_frequency (NormalizedFrequency frequency) noexcept;

	/**
	 * Fill output buffer.
	 */
	void
	fill (Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2) noexcept;

  private:
	/**
	 * Return random sample in range [-2.0..2.0] with triangular distribution.
	 */
	Sample
	noise_sample() noexcept;

	void
	update_unison_coefficients() noexcept;

	template<bool with_noise, bool unison_stereo>
		void
		fill_impl (Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2) noexcept;

  private:
	/**
	 * One unison voice params.
	 */
	struct UnisonVoice
	{
		Sample relative_frequency;
		Sample phase;
		Sample noise_level;
		Sample stereo_level_1; // Level in channel 1 (left)
		Sample stereo_level_2; // Level in channel 1 (right)
		Sample vibrato_level;
		Sample vibrato_frequency;
		Sample vibrato_phase;
	};

  private:
	bool					_wave_enabled;
	DSP::Wave*				_wave						= nullptr;
	Haruhi::AudioBuffer*	_frequency_source			= nullptr;
	Haruhi::AudioBuffer*	_amplitude_source			= nullptr;
	Haruhi::AudioBuffer*	_fm_source					= nullptr;
	Sample					_vibrato[MaxUnison];
	UnisonVoice				_unison[MaxUnison];

	// Unison:
	Sample					_initial_phase_spread		= 0;
	int						_unison_number				= -1; 		// -1 to force update unison coefficients:
	Sample					_unison_spread				= 1.0;		// Factor used to multiply input frequency: [1.0..2.0].
	Sample					_unison_noise				= 0.0;
	bool					_unison_stereo				= false;
	Sample					_unison_vibrato_level		= 0.0;
	Sample					_unison_vibrato_frequency	= 0.0;
	Sample					_1_div_unison_number;		// Cached 1.0f / _unison_number.

	// Used for both white noise and unison noise:
	DSP::Noise				_noise;
	DSP::Noise::State		_noise_state				{ _noise.state() };
	bool					_noise_enabled				= false;
	Amplitude				_noise_amplitude			= 0.0;
};


inline void
VoiceOscillator::set_wave_enabled (bool enabled) noexcept
{
	_wave_enabled = enabled;
}


inline void
VoiceOscillator::set_wave (DSP::Wave* wave) noexcept
{
	_wave = wave;
}


inline DSP::Wave*
VoiceOscillator::wave() const noexcept
{
	return _wave;
}


inline void
VoiceOscillator::set_frequency_source (Haruhi::AudioBuffer* source) noexcept
{
	_frequency_source = source;
}


inline void
VoiceOscillator::set_amplitude_source (Haruhi::AudioBuffer* source) noexcept
{
	_amplitude_source = source;
}


inline void
VoiceOscillator::set_fm_source (Haruhi::AudioBuffer* source) noexcept
{
	_fm_source = source;
}


inline void
VoiceOscillator::set_noise_enabled (bool enabled) noexcept
{
	_noise_enabled = enabled;
}


inline void
VoiceOscillator::set_noise_amplitude (Amplitude amplitude) noexcept
{
	_noise_amplitude = amplitude;
}


inline void
VoiceOscillator::set_initial_phases_spread (Sample spread) noexcept
{
	_initial_phase_spread = spread;
}


inline void
VoiceOscillator::set_unison_spread (Sample spread) noexcept
{
	if (_unison_spread != spread + 1.0f)
	{
		_unison_spread = spread + 1.0f;
		update_unison_coefficients();
	}
}


inline void
VoiceOscillator::set_unison_noise (Sample noise) noexcept
{
	_unison_noise = (1.0f / 20.f) * noise;
}


inline void
VoiceOscillator::set_unison_stereo (bool stereo) noexcept
{
	_unison_stereo = stereo;
}


inline void
VoiceOscillator::set_unison_vibrato_level (Sample level) noexcept
{
	if (_unison_vibrato_level != level)
	{
		_unison_vibrato_level = level;
		update_unison_coefficients();
	}
}


inline void
VoiceOscillator::set_unison_vibrato_frequency (NormalizedFrequency frequency) noexcept
{
	if (_unison_vibrato_frequency != frequency)
	{
		_unison_vibrato_frequency = frequency;
		update_unison_coefficients();
	}
}


inline void
VoiceOscillator::fill (Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2) noexcept
{
	assert (output_1 != 0);
	assert (output_2 != 0);
	assert (output_1->size() == output_2->size());

	Sample* const o1 = output_1->begin();
	Sample* const o2 = output_2->begin();
	bool mul = false;

	// Synthesize wave:
	if (!_wave || !_wave_enabled)
	{
		output_1->clear();
		output_2->clear();
	}
	else
	{
		if (_unison_noise > 0.0f)
		{
			if (_unison_stereo)
				fill_impl<true, true> (output_1, output_2);
			else
				fill_impl<true, false> (output_1, output_2);
		}
		else
		{
			if (_unison_stereo)
				fill_impl<false, true> (output_1, output_2);
			else
				fill_impl<false, false> (output_1, output_2);
		}
		mul = true;
	}

	// Add noise:
	if (_noise_enabled && _noise_amplitude > 0.0f)
	{
		for (std::size_t i = 0, n = output_1->size(); i < n; ++i)
		{
			float const x = static_cast<float> (_noise_amplitude * _noise.get (_noise_state));
			o1[i] += x;
			o2[i] += x;
		}
		mul = true;
	}

	if (mul)
	{
		// Multiply samples by _volume and divide samples by _unison_number:
		// (these coefficients have been obtained by listening tests):
		float amp = FastPow::pow (_1_div_unison_number, _unison_stereo ? 0.4f : 0.75f);
		SIMD::multiply_buffers_and_by_scalar (o1, o2, _amplitude_source->begin(), output_1->size(), amp);
	}
}


inline Sample
VoiceOscillator::noise_sample() noexcept
{
	return _noise.get (_noise_state) + _noise.get (_noise_state);
}


inline void
VoiceOscillator::update_unison_coefficients() noexcept
{
	float const first_frequency = 1.0f / _unison_spread;
	// Computing unison freq. multiplier:
	//   (1/a)*x^(n-1)=a => x=root(a^2, n-1),
	// where a is _unison_spread and n is _unison_number.
	float const freq_multiplier = std::pow (_unison_spread * _unison_spread, 1.0f / (_unison_number - 1));

	_1_div_unison_number = 1.0f / _unison_number;
	// Noise levels for each unison voice:
	if (_unison_number == 1)
		_unison[0].noise_level = 1.0f;
	else if (_unison_number == 2)
		_unison[0].noise_level = _unison[1].noise_level = 1.0f;
	else
	{
		for (int u = 0; u <= _unison_number / 2; ++u)
			_unison[u].noise_level = 1.0f * (u + 1) / (_unison_number / 2 + 1);
		for (int u = 0; u <= _unison_number / 2; ++u)
			_unison[_unison_number - u - 1].noise_level = _unison[u].noise_level;
	}
	// Relative frequencies and stereo spread values:
	for (int u = 0; u < _unison_number; ++u)
	{
		_unison[u].relative_frequency = u == 0
			? first_frequency
			: _unison[u - 1].relative_frequency * freq_multiplier;
		_unison[u].stereo_level_1 = pow2 (_1_div_unison_number * (u + 1));
	}
	for (int u = 0; u < _unison_number; ++u)
		_unison[u].stereo_level_2 = _unison[_unison_number - u - 1].stereo_level_1;
	// Vibrato coefficients:
	for (int u = 0; u < _unison_number; ++u)
	{
		_unison[u].vibrato_level = 0.025f * _unison_vibrato_level * _unison_spread;
		_unison[u].vibrato_frequency = _unison_vibrato_frequency * renormalize (_noise.get (_noise_state), -1.0f, 1.0f, 0.5f, 2.0f);
		limit_value (_unison[u].vibrato_frequency, 0.0f, 0.5f);
	}
}


template<bool with_noise, bool unison_stereo>
	inline void
	VoiceOscillator::fill_impl (Haruhi::AudioBuffer* output_1, Haruhi::AudioBuffer* output_2) noexcept
	{
		assert (output_1->size() == output_2->size());
		assert (output_1->size() == _amplitude_source->size());
		assert (output_1->size() == _frequency_source->size());
		assert (output_1->size() == _fm_source->size());

		Sample f, g, v, e, sum1, sum2, tmpsum;
		Sample* const o1 = output_1->begin();
		Sample* const o2 = output_2->begin();
		Sample* const fs = _frequency_source->begin();

		// Oscillate:
		for (std::size_t i = 0; i < output_1->size(); ++i)
		{
			sum1 = sum2 = 0.0f;
			if (with_noise)
				e = std::sqrt (fs[i]) * _unison_noise;
			// Add unisons:
			for (int u = 0; u < _unison_number; ++u)
			{
				g = fs[i] * _unison[u].relative_frequency;
				f = g * _fm_source->begin()[i];
				limit_value (f, 0.0f, 0.5f);
				// Unison vibrato:
				_unison[u].vibrato_phase = mod1 (_unison[u].vibrato_phase + _unison[u].vibrato_frequency);
				v = fs[i] * _unison[u].vibrato_level * DSP::base_sin<5, Sample> (_unison[u].vibrato_phase * 2.0f - 1.0f);
				// Update phases:
				if (with_noise)
					_unison[u].phase = mod1 (_unison[u].phase + v + f + e * noise_sample() * _unison[u].noise_level);
				else
					_unison[u].phase = mod1 (_unison[u].phase + v + f);
				// Don't take "noised f" as wave's frequency, because this might result in frequent jumping
				// between two waves and unwanted audible noise on some notes. It's better to get
				// some (inaudible) aliasing than that:
				tmpsum = (*_wave)(_unison[u].phase, g, i);
				// Stereo:
				if (unison_stereo)
				{
					sum1 += _unison[u].stereo_level_1 * tmpsum;
					sum2 += _unison[u].stereo_level_2 * tmpsum;
				}
				else
				{
					sum1 += tmpsum;
					sum2 += tmpsum;
				}
			}
			o1[i] = sum1;
			o2[i] = sum2;
		}
	}

} // namespace Yuki

#endif

