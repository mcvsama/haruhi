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

// Standard:
#include <cstddef>
#include <complex>
#include <cmath>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "filter_ir.h"


namespace Yuki {

FilterImpulseResponse::FilterImpulseResponse (Type type, Sample frequency, Sample resonance, Sample gain, Sample attenuation) noexcept:
	_type (type),
	_frequency (frequency),
	_resonance (resonance),
	_gain (0),
	_attenuation (attenuation),
	_limiter (false),
	_dont_update (true)
{
	set_frequency (frequency);
	set_resonance (resonance);
	set_gain (gain);
	set_attenuation (attenuation);
	_dont_update = false;
	update();
}


Sample
FilterImpulseResponse::response (Sample frequency) const noexcept
{
	// Normally one would substitute: z = e^(i2πf) = e^(iω)
	float phi = std::sin (M_PI * frequency);
	phi *= phi;
	return (std::pow (b[0] + b[1] + b[2], 2.0f) - 4.0f * (b[0] * b[1] + 4.f * b[0] * b[2] + b[1] * b[2]) * phi + 16.0f * b[0] * b[2] * std::pow (phi, 2.0f)) /
		   (std::pow (1.0f + a[1] + a[2], 2.0f) - 4.0f * (1.0f * a[1] + 4.f * 1.0f * a[2] + a[1] * a[2]) * phi + 16.0f * 1.0f * a[2] * std::pow (phi, 2.0f));
}


void
FilterImpulseResponse::update() noexcept
{
	if (_dont_update)
		return;

	float A = std::pow (10.0f, _gain / 40.0f);
	float w0 = 2.0f * M_PI * _frequency;
	float alpha = std::sin (w0) / (2.0f * _resonance);
	float cos_w0 = 0.0f;
	float sin_w0 = 0.0f;
	float sqrt_A = 0.0f;
	float sqrt_A_2_alpha = 0.0f;
	float auto_attenuation = 1.0f;

	switch (_type)
	{
		case LowPass:
			cos_w0 = std::cos (w0);

			a[0] =  1.0f + alpha;

			b[0] = (1.0f - cos_w0) / 2.0f / a[0];
			b[1] = (1.0f - cos_w0) / a[0];
			b[2] = (1.0f - cos_w0) / 2.0f / a[0];

			a[1] = -2.0f * cos_w0 / a[0];
			a[2] = (1.0f - alpha) / a[0];

			auto_attenuation = std::min (1.0f, 1.0f / _resonance);
			break;

		case HighPass:
			cos_w0 = std::cos (w0);

			a[0] =   1.0f + alpha;

			b[0] =  (1.0f + cos_w0) / 2.0f / a[0];
			b[1] = -(1.0f + cos_w0) / a[0];
			b[2] =  (1.0f + cos_w0) / 2.0f / a[0];

			a[1] =  -2.0f * cos_w0 / a[0];
			a[2] =  (1.0f - alpha) / a[0];

			auto_attenuation = std::min (1.0f, 1.0f / _resonance);
			break;

		// When limiter is enabled, this BP works as BP-const-gain.
		case BandPass:
			sin_w0 = std::sin (w0);
			cos_w0 = std::cos (w0);

			a[0] =  1.0f + alpha;

			b[0] =  sin_w0 / 2.0f / a[0];
			b[1] =  0.0f;
			b[2] = -sin_w0 / 2.0f / a[0];

			a[1] = -2.0f * cos_w0 / a[0];
			a[2] = (1.0f - alpha) / a[0];

			auto_attenuation = 1.0f / _resonance;
			break;

		case Notch:
			cos_w0 = std::cos (w0);

			a[0] =  1.0f + alpha;

			b[0] =  1.0f / a[0];
			b[1] = -2.0f * cos_w0 / a[0];
			b[2] =  1.0f / a[0];

			a[1] = b[1];
			a[2] = (1.0f - alpha) / a[0];
			break;

		case AllPass:
			cos_w0 = std::cos (w0);

			a[0] =  1.0f + alpha;

			b[0] = (1.0f - alpha) / a[0];
			b[1] = -2.0f * cos_w0 / a[0];
			b[2] =  1.0f;

			a[1] = -2.0f * cos_w0 / a[0];
			a[2] = (1.0f - alpha) / a[0];
			break;

		case Peaking:
			cos_w0 = std::cos (w0);

			a[0] =  1.0f + alpha / A;

			b[0] = (1.0f + alpha * A) / a[0];
			b[1] = -2.0f * cos_w0 / a[0];
			b[2] = (1.0f - alpha * A) / a[0];

			a[1] = -2.0f * cos_w0 / a[0];
			a[2] = (1.0f - alpha / A) / a[0];

			auto_attenuation = std::min (1.0f, 1.0f / std::pow (10.0f, _gain / 20.0f));
			break;

		case LowShelf:
			cos_w0 = std::cos (w0);
			sqrt_A = std::sqrt (A);
			sqrt_A_2_alpha = 2.0f * sqrt_A * alpha;

			a[0] =              (A + 1.0f) + (A - 1.0f) * cos_w0 + sqrt_A_2_alpha;

			b[0] =        (A * ((A + 1.0f) - (A - 1.0f) * cos_w0) + sqrt_A_2_alpha) / a[0];
			b[1] =  2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_w0) / a[0];
			b[2] =        (A * ((A + 1.0f) - (A - 1.0f) * cos_w0) - sqrt_A_2_alpha) / a[0];

			a[1] =     -2.0f * ((A - 1.0f) + (A + 1.0f) * cos_w0) / a[0];
			a[2] =             ((A + 1.0f) + (A - 1.0f) * cos_w0 - sqrt_A_2_alpha) / a[0];

			auto_attenuation = std::min (1.0f, 1.0f / _resonance / std::pow (10.0f, _gain / 20.0f));
			break;

		case HighShelf:
			cos_w0 = std::cos (w0);
			sqrt_A = std::sqrt (A);
			sqrt_A_2_alpha = 2.0f * sqrt_A * alpha;

			a[0] =              (A + 1.0f) - (A - 1.0f) * cos_w0 + sqrt_A_2_alpha;

			b[0] =         A * ((A + 1.0f) + (A - 1.0f) * cos_w0 + sqrt_A_2_alpha) / a[0];
			b[1] = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_w0) / a[0];
			b[2] =         A * ((A + 1.0f) + (A - 1.0f) * cos_w0 - sqrt_A_2_alpha) / a[0];

			a[1] =      2.0f * ((A - 1.0f) - (A + 1.0f) * cos_w0) / a[0];
			a[2] =             ((A + 1.0f) - (A - 1.0f) * cos_w0 - sqrt_A_2_alpha) / a[0];

			auto_attenuation = std::min (1.0f, 1.0f / _resonance / std::pow (10.0f, _gain / 20.0f));
			break;
	}

	// Since we normalized a[0], now reset it to 0, so it doesn't add value when convolving:
	a[0] = 0.0f;

	// Limiter off?
	if (!_limiter)
		auto_attenuation = 1.0f;

	// Apply total attenuation:
	for (int i = 0; i < 3; ++i)
		b[i] *= _attenuation * auto_attenuation;

	// Indicate that coeffs. changed:
	bump();
}

} // namespace Yuki

