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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/fft.h>
#include <haruhi/utility/exception.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "fft_filler.h"


namespace Haruhi {

namespace DSP {

FFTFiller::FFTFiller (Wave* wave, bool autoscale):
	_wave (wave),
	_autoscale (autoscale),
	_cancel_predicate (0),
	_was_interrupted (false)
{
}


void
FFTFiller::fill (Wavetable* wavetable, unsigned int samples)
{
#define CHECK_INTERRUPT do { if (interrupted()) { wavetable->drop_tables(); return; } } while (false)

	_was_interrupted = false;

	typedef std::map<float, Sample*> Tables;

	if (samples < 4096)
		throw Exception ("samples number must be at least 4096");

	Tables tables;

	wavetable->drop_tables();
	wavetable->set_wavetables_size (samples);

	// Compute max freqs. for tables:
	unsigned int const tables_num = 36; // New table about every 4 semitones
	float const expand_coeff = 1.25; // Min max frequency: ~19kHz
	for (unsigned int i = 0; i < tables_num; ++i)
		tables[0.5f - 0.5f * (std::pow (expand_coeff, i) - 1) / std::pow (expand_coeff, i)] = new Sample[samples];

	// Create wavetables:
	for (Tables::iterator t = tables.begin(); t != tables.end(); ++t)
	{
		CHECK_INTERRUPT;
		wavetable->add_table (t->second, t->first);
	}

	FFT::Vector source (samples);
	FFT::Vector target (samples);

	FFT::Forward forward (source);
	FFT::Inverse inverse (source, target);

	// Create freq. spectrum of original wave:
	Sample max = 0.0f, new_max;
	for (unsigned int i = 0; i < samples; ++i)
	{
		source[i].real() = (*_wave)(1.0f * i / (samples), 0);
		source[i].imag() = 0.0f;
		if ((new_max = std::abs (source[i].real())) > max)
			max = new_max;
	}
	if (_autoscale && max > 0.0f)
		for (unsigned int i = 0; i < samples; ++i)
			source[i].real() /= max;
	forward.transform();

	// Create wavetables by bandlimiting obtained spectrum.
	// 0 frequency is at index 0, max freq is at index samples/2.
	for (Tables::iterator t = tables.begin(); t != tables.end(); ++t)
	{
		CHECK_INTERRUPT;

		int range = (int)samples * t->first; // t->first is max_frequency

		int samples_left = 1.0f / (2.0f * t->first);
		range = bound (range, 0, static_cast<int> (samples));

		// Clear some frequencies:
		for (unsigned int i = samples_left + 1; i <= samples - samples_left; ++i)
			source[i].real() = source[i].imag() = 0.0f;
		// Transform spectrum and put it into @target:
		inverse.transform();

		// Copy result into wavetable:
		for (unsigned int i = 0; i < samples; ++i)
			t->second[i] = target[i].real();
	}

#undef CHECK_INTERRUPT
}

} // namespace DSP

} // namespace Haruhi

