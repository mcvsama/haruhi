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

#ifndef HARUHI__DSP__OSCILLATOR_H__INCLUDED
#define HARUHI__DSP__OSCILLATOR_H__INCLUDED

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


namespace Haruhi {

namespace DSP {

class Oscillator
{
	enum {
		MaxUnison = 10,
	};

  public:
	/**
	 * If oscillator has no assigned wavetable, it will
	 * mixin/fill silence.
	 */
	Oscillator (Wavetable* wavetable = 0);

	/**
	 * Sets new wavetable. May be 0.
	 */
	void
	set_wavetable (Wavetable* wavetable) { _wavetable = wavetable; }

	/**
	 * Returns currently used wavetable.
	 */
	Wavetable*
	wavetable() const { return _wavetable; }

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
	 * Argument: [0…1.0], the Nyquist frequency is at 0.5, so useful range is [0…0.5].
	 */
	void
	set_frequency (Sample frequency) { _frequency = frequency; }

	/**
	 * Buffer shoudl contain multiplier values for frequency modulation.
	 * Use 0 to disable modulator.
	 */
	void
	set_frequency_modulator (AudioBuffer* buffer) { _frequency_modulator = buffer; }

	/**
	 * Argument: [0…1.0]
	 */
	void
	set_value (Sample value) { _value = value; }

	/**
	 * Argument: [1…MaxUnison]
	 */
	void
	set_unison_number (unsigned int number);

	/**
	 * Argument: [0…1.0]
	 */
	void
	set_unison_spread (Sample spread)
	{
		if (_unison_spread != spread)
		{
			_unison_spread = spread / 20.0f;
			update_unison_coefficients();
		}
	}

	/**
	 * Argument: [0…1.0] (0.0 disables noise completely).
	 */
	void
	set_unison_noise (Sample noise) { _unison_noise = noise / 20.0f; }

	void
	reset();

	void
	fill (Sample* begin, Sample* end)
	{
		assert (begin <= end);

		std::fill (begin, end, 0.0f);

		if (_unison_noise > 0.0f)
			fill_with_noised_unison (begin, end);
		else
			fill_without_noised_unison (begin, end);

		// Multiply samples by _volume and divide samples by _unison_number:
		float amp = _value * fast_pow (_1_div_unison_number, 0.75f);
		// TODO might be optimized by using SIMD instructions:
		for (Sample* c = begin; c != end; ++c)
			*c *= amp;
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
		assert (_unison_number >= 1);
		assert (_wavetable);

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
			for (unsigned int i = 0; i <= _unison_number / 2; ++i)
				_distribution_lookup[i] = 1.0f * (i + 1) / (_unison_number / 2 + 1);
			for (unsigned int i = 0; i <= _unison_number / 2; ++i)
				_distribution_lookup[_unison_number - i - 1] = _distribution_lookup[i];
		}
	}

	void
	fill_without_noised_unison (Sample* begin, Sample* end)
	{
		Sample f;

		_x = 0;
		// Oscillate:
		for (Sample* c = begin; c != end; ++c)
		{
			_sum = 0.0f;
			_f = _frequency;
			if (_frequency_modulator)
				_f *= (*_frequency_modulator)[_x++];
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
			for (unsigned int i = 0; i < _unison_number; ++i)
			{
				// Multiply freq by 4, because frequency is in range [0…0.5], and phase is in [0…1.0]:
				_phases[i] = mod1 (_phases[i] + f);
				_sum += (*_wavetable)(_phases[i], f);
				f += _d;
			}
			*c = _sum;
		}
	}

	void
	fill_with_noised_unison (Sample* begin, Sample* end)
	{
		Sample f;

		_x = 0;
		// Oscillate:
		for (Sample* c = begin; c != end; ++c)
		{
			_sum = 0.0f;
			_f = _frequency;
			if (_frequency_modulator)
				_f *= (*_frequency_modulator)[_x++];
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
			for (unsigned int i = 0; i < _unison_number; ++i)
			{
				_z = f + _e * noise_sample() * _distribution_lookup[i];
				// Multiply freq by 4, because frequency is in range [0…0.5], and phase is in [0…1.0]:
				_phases[i] = mod1 (_phases[i] + _z);
				_sum += (*_wavetable)(_phases[i], _z);
				f += _d;
			}
			*c = _sum;
		}
	}

	/**
	 * Returns random sample in range [-2.0..2.0] with triangle distribution.
	 */
	Sample
	noise_sample()
	{
		return _noise.get (_noise_state) + _noise.get (_noise_state);
	}

  private:
	Wavetable*			_wavetable;
	Sample				_phases[MaxUnison];
	Sample				_distribution_lookup[MaxUnison];
	Sample				_frequency;
	Sample				_value;
	Sample				_initial_phase_spread;
	unsigned int		_unison_number;
	Sample				_1_div_unison_number;
	Sample				_unison_spread;
	// Cached _unison_spread / _unson_number:
	Sample				_unison_relative_spread;
	// Cached (_unison_number - 1) / 2.0f;
	Sample				_half_unison_number;
	Sample				_unison_noise;
	AudioBuffer*		_frequency_modulator;
	DSP::Noise			_noise;
	DSP::Noise::State	_noise_state;

	// Helpers:
	Sample				_sum;	// Unison waves sum (multiphases sum)
	Sample				_l;		// Unison lowest frequency
	Sample				_c;		// Unison center frequency
	Sample				_h;		// Unison highest frequency
	Sample				_e;		// Noising range (extent)
	Sample				_f;		// Frequency
	Sample				_d;		// Unison delta
	Sample				_z;		// Noised frequency
	int					_x;		// Frequency modulator sample index (used between advances())
};

} // namespace DSP

} // namespace Haruhi

#endif

