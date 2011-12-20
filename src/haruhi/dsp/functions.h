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

#ifndef HARUHI__DSP__FUNCTIONS_H__INCLUDED
#define HARUHI__DSP__FUNCTIONS_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <limits>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/dsp/parametric_wave.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/fast_pow.h>


namespace Haruhi {

namespace DSP {

/*
 * Fast versions of sine using Taylor's series.
 * Defined in [-1.0, 1.0].
 */

template<unsigned int series, class Sample>
	inline Sample
	base_sin (Sample register x)
	{
		static_assert (series == 5 || series == 6 || series == 7, InvalidArgument_series);

		if (x > 0.5)		x = +1.0 - x;
		else if (x < -0.5)	x = -1.0 - x;
		x *= M_PI;
		Sample const register xx = x * x;
		if (series == 5)
			return x * (xx * (xx * (xx * (xx * (1.0f/362880.0f) - (1.0f/5040.0f)) + (1.0f/120.0f)) - (1.0f/6.0f)) + 1.0f);
		else if (series == 6)
			return x * (xx * (xx * (xx * (xx * (xx * (1.0f/39916800.0f) + (1.0f/362880.0f)) - (1.0f/5040.0f)) + (1.0f/120.0f)) - (1.0f/6.0f)) + 1.0f);
		else if (series == 7)
			return x * (xx * (xx * (xx * (xx * (xx * (xx * (1.0f/6227020800.0f) - (1.0f/39916800.0f)) + (1.0f/362880.0f)) - (1.0f/5040.0f)) + (1.0f/120.0f)) - (1.0f/6.0f)) + 1.0f);
	}


namespace ParametricWaves {

#define HARUHI_CLONABLE(klass)		\
	ParametricWave*	clone() const {	\
		return new klass();			\
	}

	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Sine: public ParametricWave
	{
	  public:
		HARUHI_CLONABLE (Sine)

		Sine():
			ParametricWave (true)
		{ }

		Sample
		operator() (Sample x, Sample) const
		{
			x = x * 2.0f - 1.0f;
			float a = pow2 (pow2 (1.0f + param())); // x^4
			float sgn = x >= 0.0f ? 1.0f : -1.0f;
			return sgn * (DSP::base_sin<7> (std::pow (sgn * x, a)));
		}
	};


	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Triangle: public ParametricWave
	{
	  public:
		HARUHI_CLONABLE (Triangle)

		Triangle():
			ParametricWave (true)
		{ }

		Sample
		operator() (Sample x, Sample) const
		{
			x = x * 2.0f - 1.0f;
			float sgn = 1.0f;
			if (x < 0.0f)
			{
				x = -x;
				sgn = -1.0f;
			}
			if (x > 0.5f)
				x = 1.0f - x;
			x = 1.0f / std::max ((1.0f - param()) / 2.0f, 0.000001f) * x;
			if (x > 1.0)
				x = 1.0;
			return sgn * x;
		}
	};


	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Square: public ParametricWave
	{
	  public:
		HARUHI_CLONABLE (Square)

		Square():
			ParametricWave (true)
		{ }

		Sample
		operator() (Sample x, Sample) const
		{
			x = x * 2.0f - 1.0f;
			float sgn = 1.0f;
			if (x < 0.0f)
			{
				x = -x;
				sgn = -1.0f;
			}
			x = x < (1.001f - param()) ? 1.0f : 0.0f;
			return sgn * x;
		}
	};


	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Sawtooth: public ParametricWave
	{
	  public:
		HARUHI_CLONABLE (Sawtooth)

		Sawtooth():
			ParametricWave (true)
		{ }

		Sample
		operator() (Sample x, Sample) const
		{
			x = mod1 (x);
			float a = param();
			if (a < 0.00001f)
				a = 0.00001f;
			else if (a > 0.99999f)
				a = 0.99999f;
			if (x < a)
				return x / a * 2.0f - 1.0f;
			else
				return (1.0f - x) / (1.0f - a) * 2.0f - 1.0f;
		}
	};


	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Pulse: public ParametricWave
	{
	  public:
		HARUHI_CLONABLE (Pulse)

		Pulse():
			ParametricWave (true)
		{ }

		Sample
		operator() (Sample x, Sample) const
		{
			float a = param();
			if (a < 0.005f)
				a = 0.005f;
			else if (a > 0.995f)
				a = 0.995f;
			return x > a ? -1.0f : 1.0f;
		}
	};


	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Power: public ParametricWave
	{
	  public:
		HARUHI_CLONABLE (Power)

		Power():
			ParametricWave (true)
		{ }

		Sample
		operator() (Sample x, Sample) const
		{
			x = x * 2.0f - 1.0f;
			Sample sgn = x >= 0.0f ? 1.0f : -1.0f;
			return sgn * FastPow::pow (mod1 (sgn * x), 50.0f * param() * param() * param());
		}
	};


	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Gauss: public ParametricWave
	{
	  public:
		HARUHI_CLONABLE (Gauss)

		Gauss():
			ParametricWave (true)
		{ }

		Sample
		operator() (Sample x, Sample) const
		{
			float a = param();
			x = mod1 (x) * 2.0f - 1.0f;
			if (a < 0.00001f)
				a = 0.00001f;
			return std::exp (-x * x * (std::exp (a * 8.0f) + 5.0f)) * 2.0f - 1.0f;
		}
	};


	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Diode: public ParametricWave
	{
	  public:
		HARUHI_CLONABLE (Diode)

		Diode():
			ParametricWave (true)
		{ }

		Sample
		operator() (Sample x, Sample) const
		{
			float a = param();
			if (a < 0.001f)
				a = 0.001f;
			else if (a > 0.999f)
				a = 0.999f;
			a = a * 2.0f - 1.0f;
			x = std::cos ((x + 0.5f) * 2.0f * M_PI) - a;
			if (x < 0.0f)
				x = 0.0f;
			return x / (1.0f - a) * 2.0f - 1.0f;
		}
	};


	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Chirp: public ParametricWave
	{
	  public:
		HARUHI_CLONABLE (Chirp)

		Chirp():
			ParametricWave (true)
		{ }

		Sample
		operator() (Sample x, Sample) const
		{
			float a = renormalize (param(), 0.0f, 1.0f, 0.25f, 1.0f);
			x = mod1 (x) * 2.0f * M_PI;
			a = (a - 0.5f) * 4.0f;
			if (a < 0.0f)
				a *= 2.0f;
			a = FastPow::pow (3.0f, a);
			return std::sin (x / 2.0f) * std::sin (a * x * x);
		}
	};

#undef HARUHI_CLONABLE

} // namespace ParametricWaves


namespace SeriesFunctions {

/**
 * Argument: [-1.0, 1.0].
 */
template<unsigned int series, class Sample>
	inline Sample
	base_sin (Sample register x, Sample base_frequency, Sample fmin, Sample fmax)
	{
		if (base_frequency >= fmin)
			return DSP::base_sin<series> (x);
		return 0.0;
	}


/*
 * Generates bandlimited triangle wave
 * for frequency base_frequency limited within [fmin, fmax).
 *
 * f(x) = 8/π² ∑_{k=1}^∞ (-1)^(k-1) (sin ((2k-1)x) / (2k-1)²)
 *
 * Preconditions:
 *  ▪ base_frequency > 0
 *  ▪ fmin >= base_frequency
 *  ▪ fmax >= fmin
 *
 * Argument: [-1.0, 1.0].
 */
template<class Sample>
	inline Sample
	triangle (Sample register x, Sample base_frequency, Sample fmin, Sample fmax)
	{
		Sample register y = 0;

		const int n = static_cast<int> (std::floor (0.5f * fmax / base_frequency));
		const int a = static_cast<int> (std::ceil (0.5f * fmin / base_frequency));

		float sgn = (a % 2) ? -1.0 : 1.0;
		for (int register k = std::max (1, a); k <= n; ++k)
		{
			float z = std::fmod ((2 * k - 1) * (x + 1.f), 2.f) - 1.f;
			y += sgn * DSP::base_sin<5> (z) / pow2<Sample> (2 * k - 1);
			sgn = -sgn;
		}

		return y * 8.0f / (M_PI * M_PI);
	}


/*
 * Generates bandlimited square wave
 * for frequency base_frequency limited within [fmin, fmax).
 *
 * f(x) = 4/π ∑_{k=1}^∞ (sin ((2k-1)x) / (2k-1))
 *
 * Preconditions:
 *  ▪ base_frequency > 0
 *  ▪ fmin >= base_frequency
 *  ▪ fmax >= fmin
 *
 * Argument: [-1.0, 1.0].
 */
template<class Sample>
	inline Sample
	square (Sample register x, Sample base_frequency, Sample fmin, Sample fmax)
	{
		Sample register y = 0;

		const int n = static_cast<int> (std::floor (0.5f * fmax / base_frequency));
		const int a = static_cast<int> (std::ceil (0.5f * fmin / base_frequency));

		for (int register k = std::max (1, a); k <= n; ++k)
			y += DSP::base_sin<5> (std::fmod ((2 * k - 1) * (x + 1.0f), 2.0f) - 1.0f) / static_cast<Sample> (2 * k - 1);

		return y * 4.0f / M_PI;
	}


/*
 * Generates bandlimited sawtooth wave
 * for frequency base_frequency limited within [fmin, fmax).
 *
 * f(x) = 2/π ∑_{k=1}^∞ (sin (kx) / k)
 *
 * Preconditions:
 *  ▪ base_frequency > 0
 *  ▪ fmin >= base_frequency
 *  ▪ fmax >= fmin
 *
 * Argument: [-1.0, 1.0].
 */
template<class Sample>
	inline Sample
	sawtooth (Sample register x, Sample base_frequency, Sample fmin, Sample fmax)
	{
		Sample register y = 0;

		const int n = static_cast<int> (std::floor (fmax / base_frequency));
		const int a = static_cast<int> (std::ceil (fmin / base_frequency));

		for (int register k = std::max (1, a); k <= n; ++k)
			y += DSP::base_sin<5> (std::fmod (k * (x + 1.0f), 2.0f) - 1.0f) / static_cast<Sample> (k);

		return y * M_2_PI;
	}

} // namespace SeriesFunctions

} // namespace DSP

} // namespace Haruhi

#endif

