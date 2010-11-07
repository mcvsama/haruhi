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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/exception.h>
#include <haruhi/dsp/fft.h>

// Local:
#include "fft_filler.h"


namespace Haruhi {

namespace DSP {

FFTFiller::FFTFiller (Wave* wave, bool autoscale):
	_wave (wave),
	_autoscale (autoscale)
{
}


void
FFTFiller::fill (Wavetable* wavetable, unsigned int samples)
{
	if (samples < 4096)
		throw new Exception ("samples number must be at least 4096");

	const unsigned int oversampling = 1;
	const unsigned int number = Wavetable::Number;

	wavetable->drop_tables();
	wavetable->set_wavetables_size (samples);
	Wavetable::Tables& tables = wavetable->tables();

	unsigned int prev_range = samples * oversampling / 2;
	unsigned int range;

	// Create wavetables:
	for (unsigned int i = 0; i < number; ++i)
		tables.push_back (new Core::Sample[samples]);

	FFT::Vector source (samples * oversampling);
	FFT::Vector target (samples * oversampling);

	FFT::Forward forward (source);
	FFT::Inverse inverse (source, target);

	// Create freq. spectrum of original wave:
	Core::Sample max = 0.0f, new_max;
	for (unsigned int i = 0; i < samples * oversampling; ++i)
	{
		source[i].real() = (*_wave)(1.0f * i / (samples * oversampling), 0);
		source[i].imag() = 0.0f;
		if ((new_max = std::abs (source[i].real())) > max)
			max = new_max;
	}
	if (_autoscale && max > 0.0f)
		for (unsigned int i = 0; i < samples * oversampling; ++i)
			source[i].real() /= max;
	forward.transform();

	// Create wavetables from obtained spectrum:
	for (unsigned int t = 0; t < number; ++t)
	{
		double const base_frequency = 0.5f - 0.5f * (std::pow (2, t) - 1) / std::pow (2, t);
		range = 4096 * base_frequency;

		// Clear some frequencies according to base_frequency:
		for (unsigned int i = 1 + range; i < 1 + prev_range; ++i)
			source[i].real() = source[i].imag() = 0.0f;
		for (unsigned int i = samples * oversampling - prev_range; i < samples * oversampling - range; ++i)
			source[i].real() = source[i].imag() = 0.0f;
		prev_range = range;

		// Transform spectrum and put it into @target:
		inverse.transform();

		// Copy result into wavetable:
		for (unsigned int i = 0; i < samples; ++i)
			tables[number - 1 - t][i] = target[i * oversampling].real();
	}
}

} // namespace DSP

} // namespace Haruhi

