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
#include <complex>
#include <cmath>

// Local:
#include "rbj_impulse_response.h"


namespace MikuruPrivate {

RBJImpulseResponse::RBJImpulseResponse (Type type, Sample frequency, Sample resonance, Sample gain, Sample attenuation):
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


void
RBJImpulseResponse::set_type (Type type)
{
	if (_type != type)
	{
		_type = type;
		update();
	}
}


void
RBJImpulseResponse::set_frequency (Sample frequency)
{
	// Bound frequency to 32Hz…23.99kHz for fs=48kHz
	frequency = std::max (0.0006666666f, std::min (frequency, 0.4997916666f));
	if (_frequency != frequency)
	{
		_frequency = frequency;
		update();
	}
}


void
RBJImpulseResponse::set_resonance (Sample resonance)
{
	// Q must be greater than 0:
	resonance = std::max (0.01f, resonance);
	if (_resonance != resonance)
	{
		_resonance = resonance;
		update();
	}
}


void
RBJImpulseResponse::set_gain (Sample gain)
{
	if (_gain != gain)
	{
		_gain = gain;
		update();
	}
}


void
RBJImpulseResponse::set_attenuation (Sample attenuation)
{
	if (_attenuation != attenuation)
	{
		_attenuation = attenuation;
		update();
	}
}


void
RBJImpulseResponse::set_limiter (bool enabled)
{
	if (_limiter != enabled)
	{
		_limiter = enabled;
		update();
	}
}


Core::Sample
RBJImpulseResponse::response (Sample frequency) const
{
	// Normally one would substitute: z = e^(i2πf) = e^(iω)
	double phi = std::pow (std::sin (M_PI * frequency), 2.0);
	return (std::pow (_b[0] + _b[1] + _b[2], 2.0) - 4.0 * (_b[0] * _b[1] + 4 * _b[0] * _b[2] + _b[1] * _b[2]) * phi + 16.0 * _b[0] * _b[2] * std::pow (phi, 2.0)) /
		   (std::pow (  1.0 + _a[1] + _a[2], 2.0) - 4.0 * (  1.0 * _a[1] + 4 *   1.0 * _a[2] + _a[1] * _a[2]) * phi + 16.0 *   1.0 * _a[2] * std::pow (phi, 2.0));
}


void
RBJImpulseResponse::update()
{
	if (_dont_update)
		return;

	float A = std::pow (10.0f, _gain / 40.0f);
	float w0 = 2.0f * M_PI * _frequency;
	float alpha = std::sin (w0) / (2.0f * _resonance);
	float cos_w0 = 0.0f;
	float sin_w0 = 0.0f;
	float sqrt_A = 0.0f;
	float auto_attenuation = 1.0f;

	switch (_type)
	{
		case LowPass:
			cos_w0 = std::cos (w0);

			_a[0] =  1.0f + alpha;

			_b[0] = (1.0f - cos_w0) / 2.0f / _a[0];
			_b[1] = (1.0f - cos_w0) / _a[0];
			_b[2] = (1.0f - cos_w0) / 2.0f / _a[0];

			_a[1] = -2.0f * cos_w0 / _a[0];
			_a[2] = (1.0f - alpha) / _a[0];

			auto_attenuation = std::min (1.0f, 1.0f / _resonance);
			break;

		case HighPass:
			cos_w0 = std::cos (w0);

			_a[0] =   1.0f + alpha;

			_b[0] =  (1.0f + cos_w0) / 2.0f / _a[0];
			_b[1] = -(1.0f + cos_w0) / _a[0];
			_b[2] =  (1.0f + cos_w0) / 2.0f / _a[0];

			_a[1] =  -2.0f * cos_w0 / _a[0];
			_a[2] =  (1.0f - alpha) / _a[0];

			auto_attenuation = std::min (1.0f, 1.0f / _resonance);
			break;

		// When limiter is enabled, this BP works as BP-const-gain.
		case BandPass:
			sin_w0 = std::sin (w0);
			cos_w0 = std::cos (w0);

			_a[0] =  1.0f + alpha;

			_b[0] =  sin_w0 / 2.0f / _a[0];
			_b[1] =  0.0f;
			_b[2] = -sin_w0 / 2.0f / _a[0];

			_a[1] = -2.0f * cos_w0 / _a[0];
			_a[2] = (1.0f - alpha) / _a[0];

			auto_attenuation = 1.0f / _resonance;
			break;

		case Notch:
			cos_w0 = std::cos (w0);

			_a[0] =  1.0f + alpha;

			_b[0] =  1.0f / _a[0];
			_b[1] = -2.0f * cos_w0 / _a[0];
			_b[2] =  1.0f / _a[0];

			_a[1] = _b[1];
			_a[2] = (1.0f - alpha) / _a[0];
			break;

		case AllPass:
			cos_w0 = std::cos (w0);

			_a[0] =  1.0f + alpha;

			_b[0] = (1.0f - alpha) / _a[0];
			_b[1] = -2.0f * cos_w0 / _a[0];
			_b[2] =  1.0f;

			_a[1] = -2.0f * cos_w0 / _a[0];
			_a[2] = (1.0f - alpha) / _a[0];
			break;

		case Peaking:
			cos_w0 = std::cos (w0);

			_a[0] =  1.0f + alpha / A;

			_b[0] = (1.0f + alpha * A) / _a[0];
			_b[1] = -2.0f * cos_w0 / _a[0];
			_b[2] = (1.0f - alpha * A) / _a[0];

			_a[1] = -2.0f * cos_w0 / _a[0];
			_a[2] = (1.0f - alpha / A) / _a[0];
			break;

		case LowShelf:
			cos_w0 = std::cos (w0);
			sqrt_A = std::sqrt (A);

			_a[0] =              (A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrt_A * alpha;

			_b[0] =        (A * ((A + 1.0f) - (A - 1.0f) * cos_w0) + 2.0f * sqrt_A * alpha) / _a[0];
			_b[1] =  2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_w0) / _a[0];
			_b[2] =        (A * ((A + 1.0f) - (A - 1.0f) * cos_w0) - 2.0f * sqrt_A * alpha) / _a[0];

			_a[1] =     -2.0f * ((A - 1.0f) + (A + 1.0f) * cos_w0) / _a[0];
			_a[2] =             ((A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrt_A * alpha) / _a[0];

			auto_attenuation = std::min (1.0f, 1.0f / _resonance);
			break;

		case HighShelf:
			cos_w0 = std::cos (w0);
			sqrt_A = std::sqrt (A);

			_a[0] =              (A + 1.0f) - (A - 1.0f) * cos_w0 + 2.0f * sqrt_A * alpha;

			_b[0] =         A * ((A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrt_A * alpha) / _a[0];
			_b[1] = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_w0) / _a[0];
			_b[2] =         A * ((A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrt_A * alpha) / _a[0];

			_a[1] =      2.0f * ((A - 1.0f) - (A + 1.0f) * cos_w0) / _a[0];
			_a[2] =             ((A + 1.0f) - (A - 1.0f) * cos_w0 - 2.0f * sqrt_A * alpha) / _a[0];

			auto_attenuation = std::min (1.0f, 1.0f / _resonance);
			break;
	}

	// Since we normalized _a[0], now reset it to 0, so it doesn't add value when convolving:
	_a[0] = 0.0f;

	// Limiter off?
	if (!_limiter)
		auto_attenuation = 1.0f;

	// Apply total attenuation:
	for (int i = 0; i < 3; ++i)
		_b[i] *= _attenuation * auto_attenuation;

	// Indicate that coeffs. changed:
	bump();
}

} // namespace MikuruPrivate

