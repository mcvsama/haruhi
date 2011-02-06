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

#ifndef HARUHI__UNITS__MIKURU__DSP__OSCILLATAOR_H__INCLUDED
#define HARUHI__UNITS__MIKURU__DSP__OSCILLATAOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/wavetable.h>
#include <haruhi/dsp/noise.h>
#include <haruhi/utility/numeric.h>


namespace MikuruPrivate {

namespace DSP = Haruhi::DSP;

using Haruhi::Sample;

class VoiceOscillator 
{
	enum {
		MaxUnison = 10,
	};

  public:
	/**
	 * If oscillator has no assigned wavetable, it will
	 * mixin/fill silence.
	 */
	VoiceOscillator (DSP::Wavetable* wavetable = 0);

	/**
	 * Enables/disables wavetable generator.
	 */
	void
	set_wavetable_enabled (bool enabled) { _wavetable_enabled = enabled; }

	/**
	 * Sets new wavetable. May be 0.
	 */
	void
	set_wavetable (DSP::Wavetable* wavetable) { _wavetable = wavetable; }

	/**
	 * Returns currently used wavetable.
	 */
	DSP::Wavetable*
	wavetable() const { return _wavetable; }

	/**
	 * Sets frequency source buffer.
	 */
	void
	set_frequency_source (Haruhi::AudioBuffer* source) { _frequency_source = source; }

	/**
	 * Sets amplitude source buffer.
	 */
	void
	set_amplitude_source (Haruhi::AudioBuffer* source) { _amplitude_source = source; }

	/**
	 * Enables/disables noise generator.
	 */
	void
	set_noise_enabled (bool enabled) { _noise_enabled = enabled; }

	/**
	 * Sets noise amplitude.
	 */
	void
	set_noise_amplitude (Sample amplitude) { _noise_amplitude = amplitude; }

	/**
	 * Argument: [-1.0…1.0]
	 */
	void
	set_phase (Sample phase);

	/**
	 * Argument: [-1.0…1.0]
	 */
	void
	set_initial_phases_spread (Sample spread) { _initial_phase_spread = spread; }

	/**
	 * Argument: [1…MaxUnison]
	 */
	void
	set_unison_number (int number);

	/**
	 * Argument: [0…1.0]
	 */
	void
	set_unison_spread (Sample spread)
	{
		if (_unison_spread != spread)
		{
			_unison_spread = (1.0f / 20.0f) * spread;
			update_unison_coefficients();
		}
	}

	/**
	 * Argument: [0…1.0] (0.0 disables noise completely).
	 */
	void
	set_unison_noise (Sample noise) { _unison_noise = (1.0f / 20.f) * noise; }

	/**
	 * Enables/disables stereo spread of unison voices.
	 */
	void
	set_unison_stereo (bool stereo) { _unison_stereo = stereo; }

	/**
	 * Fills output buffer.
	 */
	void
	fill (Haruhi::AudioBuffer* output1, Haruhi::AudioBuffer* output2)
	{
		assert (output1 != 0);
		assert (output2 != 0);
		assert (output1->size() == output2->size());

		Sample* const o1 = output1->begin();
		Sample* const o2 = output2->begin();
		bool mul = false;

		// Synthesize wave:
		if (_wavetable == 0 || !_wavetable_enabled)
		{
			output1->clear();
			output2->clear();
		}
		else
		{
			if (_unison_noise > 0.0f)
				fill_with_noised_unison (output1, output2);
			else
				fill_without_noised_unison (output1, output2);
			mul = true;
		}

		// Add noise:
		if (_noise_enabled && _noise_amplitude > 0.0f)
		{
			for (std::size_t i = 0, n = output1->size(); i < n; ++i)
			{
				float const x = _noise_amplitude * _noise.get (_noise_state);
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
			SIMD::multiply_buffers_and_by_scalar (o1, o2, _amplitude_source->begin(), output1->size(), amp);
		}
	}

  private:
	Sample
	unison_delta (Sample const& f)
	{
		return f * _unison_relative_spread;
	}

	void
	update_unison_coefficients()
	{
		_unison_relative_spread = _unison_spread / _unison_number;
		_half_unison_number = (_unison_number - 1) / 2.0f;
		_1_div_unison_number = 1.0f / _unison_number;
		// Distribution lookup table:
		if (_unison_number == 1)
			_distribution_lookup[0] = 1.0f;
		else if (_unison_number == 2)
			_distribution_lookup[0] = _distribution_lookup[1] = 1.0f;
		else
		{
			for (int i = 0; i <= _unison_number / 2; ++i)
				_distribution_lookup[i] = 1.0f * (i + 1) / (_unison_number / 2 + 1);
			for (int i = 0; i <= _unison_number / 2; ++i)
				_distribution_lookup[_unison_number - i - 1] = _distribution_lookup[i];
		}
	}

	void
	initialize_stereo_unison_lookup()
	{
		// Lookup table for stereo unison:
		for (int p = 0; p < _unison_number; ++p)
			_stereo_unison_lookup[p] = pow2 (_1_div_unison_number * (p + 1));
	}


	void
	fill_without_noised_unison (Haruhi::AudioBuffer* output1, Haruhi::AudioBuffer* output2)
	{
		Sample f, d, l, h, sum1, sum2, tmpsum;
		Sample* const o1 = output1->begin();
		Sample* const o2 = output2->begin();
		Sample* const fs = _frequency_source->begin();

		initialize_stereo_unison_lookup();

		// Oscillate:
		for (std::size_t i = 0, n = output1->size(); i < n; ++i)
		{
			sum1 = sum2 = 0.0f;
			f = fs[i];
			d = unison_delta (f);
			l = f - d * _half_unison_number;
			h = f + f - l;
			f = l;
			limit_value (f, 0.0f, 0.5f);
			// Add unisons:
			if (_unison_stereo)
			{
				for (int p = 0; p < _unison_number; ++p)
				{
					_phases[p] = mod1 (_phases[p] + f);
					tmpsum = (*_wavetable)(_phases[p], f);
					// Stereo:
					sum1 += _stereo_unison_lookup[p] * tmpsum;
					sum2 += _stereo_unison_lookup[_unison_number - p - 1] * tmpsum;
					f += d;
				}
			}
			else
			{
				for (int p = 0; p < _unison_number; ++p)
				{
					_phases[p] = mod1 (_phases[p] + f);
					tmpsum = (*_wavetable)(_phases[p], f);
					sum1 += tmpsum;
					sum2 += tmpsum;
					f += d;
				}
			}
			o1[i] = sum1;
			o2[i] = sum2;
		}
	}

	void
	fill_with_noised_unison (Haruhi::AudioBuffer* output1, Haruhi::AudioBuffer* output2)
	{
		Sample e, f, d, l, h, z, sum1, sum2, tmpsum;
		Sample* const o1 = output1->begin();
		Sample* const o2 = output2->begin();
		Sample* const fs = _frequency_source->begin();

		initialize_stereo_unison_lookup();

		// Oscillate:
		for (std::size_t i = 0, n = output1->size(); i < n; ++i)
		{
			sum1 = sum2 = 0.0f;
			f = fs[i];
			e = std::sqrt (f) * _unison_noise;
			d = unison_delta (f);
			l = f - d * _half_unison_number;
			h = f + f - l;
			f = l;
			limit_value (f, 0.0f, 0.5f);
			// Add unisons:
			if (_unison_stereo)
			{
				for (int p = 0; p < _unison_number; ++p)
				{
					z = f + e * noise_sample() * _distribution_lookup[p];
					_phases[p] = mod1 (_phases[p] + z);
					// Don't take z as wave's frequency, because this might result in frequent jumping
					// between two wavetables and unwanted audible noise on some notes. It's better to get
					// some (inaudible) aliasing than that:
					tmpsum = (*_wavetable)(_phases[p], f);
					// Stereo:
					sum1 += _stereo_unison_lookup[p] * tmpsum;
					sum2 += _stereo_unison_lookup[_unison_number - p - 1] * tmpsum;
					f += d;
				}
			}
			else
			{
				for (int p = 0; p < _unison_number; ++p)
				{
					z = f + e * noise_sample() * _distribution_lookup[p];
					_phases[p] = mod1 (_phases[p] + z);
					// Don't take z as wave's frequency, because this might result in frequent jumping
					// between two wavetables and unwanted audible noise on some notes. It's better to get
					// some (inaudible) aliasing than that:
					tmpsum = (*_wavetable)(_phases[p], f);
					sum1 += tmpsum;
					sum2 += tmpsum;
					f += d;
				}
			}
			o1[i] = sum1;
			o2[i] = sum2;
		}
	}

	/**
	 * Returns random sample in range [-2.0..2.0] with triangular distribution.
	 */
	Sample
	noise_sample()
	{
		return _noise.get (_noise_state) + _noise.get (_noise_state);
	}

  private:
	bool					_wavetable_enabled;
	DSP::Wavetable*			_wavetable;
	Haruhi::AudioBuffer*	_frequency_source;
	Haruhi::AudioBuffer*	_amplitude_source;
	Sample					_distribution_lookup[MaxUnison];
	Sample					_stereo_unison_lookup[MaxUnison];
	Sample					_phases[MaxUnison];

	// Unison:
	Sample					_initial_phase_spread;
	int						_unison_number;
	Sample					_unison_spread;
	Sample					_unison_noise;
	bool					_unison_stereo;
	Sample					_1_div_unison_number;		// Cached 1.0f / _unison_number.
	Sample					_unison_relative_spread;	// Cached _unison_spread / _unson_number.
	Sample					_half_unison_number;		// Cached (_unison_number - 1) / 2.0f.

	// Used for both white noise and unison noise:
	DSP::Noise				_noise;
	DSP::Noise::State		_noise_state;
	bool					_noise_enabled;
	Sample					_noise_amplitude;
};

} // namespace MikuruPrivate

#endif

