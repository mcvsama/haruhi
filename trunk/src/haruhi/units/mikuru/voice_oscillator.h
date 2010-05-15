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

#ifndef HARUHI__UNITS__MIKURU__DSP__OSCILLATAOR_H__INCLUDED
#define HARUHI__UNITS__MIKURU__DSP__OSCILLATAOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>

// Haruhi:
#include <haruhi/core/audio.h>
#include <haruhi/core/audio_buffer.h>
#include <haruhi/dsp/wavetable.h>
#include <haruhi/dsp/noise.h>
#include <haruhi/utility/numeric.h>


namespace MikuruPrivate {

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
	 * Returns frequency source buffer.
	 */
	void
	set_frequency_source (Core::AudioBuffer* source) { _frequency_source = source; }

	/**
	 * Returns amplitude source buffer.
	 */
	void
	set_amplitude_source (Core::AudioBuffer* source) { _amplitude_source = source; }

	/**
	 * Argument: [-1.0…1.0]
	 */
	void
	set_phase (Core::Sample phase);

	/**
	 * Argument: [-1.0…1.0]
	 */
	void
	set_initial_phases_spread (Core::Sample spread) { _initial_phase_spread = spread; }

	/**
	 * Argument: [1…MaxUnison]
	 */
	void
	set_unison_number (int number);

	/**
	 * Argument: [0…1.0]
	 */
	void
	set_unison_spread (Core::Sample spread)
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
	set_unison_noise (Core::Sample noise) { _unison_noise = (1.0f / 20.f) * noise; }

	/**
	 * Fills output buffer.
	 */
	void
	fill (Core::AudioBuffer* output)
	{
		if (_wavetable == 0)
			std::fill (output->begin(), output->end(), 0.0f);
		else
		{
			if (_unison_noise > 0.0f)
				fill_with_noised_unison (output);
			else
				fill_without_noised_unison (output);

			// Multiply samples by _volume and divide samples by _unison_number:
			float amp = std::pow (_1_div_unison_number, 0.75f);
			Core::Sample* const o = output->begin();

			// TODO might be optimized by using SIMD instructions:
			for (std::size_t i = 0, n = output->size(); i < n; ++i)
				o[i] *= amp * (*_amplitude_source)[i];
		}
	}

  private:
	Core::Sample
	unison_delta (Core::Sample const& f)
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
	fill_without_noised_unison (Core::AudioBuffer* output)
	{
		Core::Sample f;
		Core::Sample* const o = output->begin();
		Core::Sample* const fs = _frequency_source->begin();

		// Oscillate:
		for (std::size_t i = 0, n = output->size(); i < n; ++i)
		{
			_sum = 0.0f;
			_f = fs[i];
			_d = unison_delta (_f);
			_l = _f - _d * _half_unison_number;
			_c = _f;
			_h = _c + _c - _l;
			f = _l;
			if (f < 0.0f)
				f = 0.0f;
			if (f > 0.5f)
				f = 0.5f;
			// Add unisons:
			for (int p = 0; p < _unison_number; ++p)
			{
				_phases[p] = mod1 (_phases[p] + f);
				_sum += (*_wavetable)(_phases[p], f);
				f += _d;
			}
			o[i] = _sum;
		}
	}

	void
	fill_with_noised_unison (Core::AudioBuffer* output)
	{
		Core::Sample f;
		Core::Sample* const o = output->begin();
		Core::Sample* const fs = _frequency_source->begin();

		// Oscillate:
		for (std::size_t i = 0, n = output->size(); i < n; ++i)
		{
			_sum = 0.0f;
			_f = fs[i];
			_e = std::sqrt (_f) * _unison_noise;
			_d = unison_delta (_f);
			_l = _f - _d * _half_unison_number;
			_c = _f;
			_h = _c + _c - _l;
			f = _l;
			if (f < 0.0f)
				f = 0.0f;
			if (f > 0.5f)
				f = 0.5f;
			// Add unisons:
			for (int p = 0; p < _unison_number; ++p)
			{
				_z = f + _e * noise_sample() * _distribution_lookup[p];
				_phases[p] = mod1 (_phases[p] + _z);
				// Don't take _z as wave's frequency, because this might result in frequent jumping
				// between two wavetables and unwanted audible white-noise:
				_sum += (*_wavetable)(_phases[p], f);
				f += _d;
			}
			o[i] = _sum;
		}
	}

	/**
	 * Returns random sample in range [-2.0..2.0] with triangular distribution.
	 */
	Core::Sample
	noise_sample()
	{
		return _noise.get (_noise_state) + _noise.get (_noise_state);
	}

  private:
	DSP::Wavetable*		_wavetable;
	Core::AudioBuffer*	_frequency_source;
	Core::AudioBuffer*	_amplitude_source;
	Core::Sample		_distribution_lookup[MaxUnison];
	Core::Sample		_phases[MaxUnison];

	// Unison:
	Core::Sample		_initial_phase_spread;
	int					_unison_number;
	Core::Sample		_unison_spread;
	Core::Sample		_unison_noise;
	Core::Sample		_1_div_unison_number;		// Cached 1.0f / _unison_number.
	Core::Sample		_unison_relative_spread;	// Cached _unison_spread / _unson_number.
	Core::Sample		_half_unison_number;		// Cached (_unison_number - 1) / 2.0f.

	DSP::Noise			_noise;
	DSP::Noise::State	_noise_state;

	// Helpers:
	Core::Sample		_sum;	// Unison waves sum (multiphases sum)
	Core::Sample		_l;		// Unison lowest frequency
	Core::Sample		_c;		// Unison center frequency
	Core::Sample		_h;		// Unison highest frequency
	Core::Sample		_e;		// Noising range (extent)
	Core::Sample		_f;		// Frequency
	Core::Sample		_d;		// Unison delta
	Core::Sample		_z;		// Noised frequency
};

} // namespace MikuruPrivate

#endif

