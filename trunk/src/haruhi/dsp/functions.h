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

#ifndef HARUHI__DSP__FUNCTIONS_H__INCLUDED
#define HARUHI__DSP__FUNCTIONS_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <limits>

// Haruhi:
#include <haruhi/dsp/wave.h>
#include <haruhi/dsp/parametric_wave.h>
#include <haruhi/utility/numeric.h>


namespace DSP {

/*
 * Fast versions of sine using Taylor's series.
 * Defined in [-1.0, 1.0].
 */

template<class Sample>
	inline Sample
	base_sine5 (Sample register x)
	{
		if (x > 0.5)		x = +1.0 - x;
		else if (x < -0.5)	x = -1.0 - x;
		x *= M_PI;
		Sample const register xx = x * x;
		return x * (xx * (xx * (xx * (xx * (1.0/362880.0) - (1.0/5040.0)) + (1.0/120.0)) - (1.0/6.0)) + 1.0);
	}


template<class Sample>
	inline Sample
	base_sine6 (Sample register x)
	{
		if (x > 0.5f)		x = +1.0f - x;
		else if (x < -0.5f)	x = -1.0f - x;
		x *= M_PI;
		Sample const register xx = x * x;
		return x * (xx * (xx * (xx * (xx * (xx * (1.0/39916800.0) + (1.0/362880.0)) - (1.0/5040.0)) + (1.0/120.0)) - (1.0/6.0)) + 1.0);
	}


template<class Sample>
	inline Sample
	base_sine7 (Sample register x)
	{
		if (x > 0.5)		x = +1.0 - x;
		else if (x < -0.5)	x = -1.0 - x;
		x *= M_PI;
		Sample const register xx = x * x;
		return x * (xx * (xx * (xx * (xx * (xx * (xx * (1.0/6227020800.0) - (1.0/39916800.0)) + (1.0/362880.0)) - (1.0/5040.0)) + (1.0/120.0)) - (1.0/6.0)) + 1.0);
	}


namespace ParametricWaves {

	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Sine: public ParametricWave
	{
	  public:
		Core::Sample
		operator() (Core::Sample x, Core::Sample) const
		{
			x = x * 2.0f - 1.0f;
			float a = std::pow (1.0f + param(), 4.0f);
			float sgn = x >= 0.0 ? 1.0f : -1.0f;
			return sgn * (DSP::base_sine7 (std::pow (sgn * x, a)));
		}
	};


	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Triangle: public ParametricWave
	{
	  public:
		Core::Sample
		operator() (Core::Sample x, Core::Sample) const
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
		Core::Sample
		operator() (Core::Sample x, Core::Sample) const
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
		Core::Sample
		operator() (Core::Sample x, Core::Sample) const
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
		Core::Sample
		operator() (Core::Sample x, Core::Sample) const
		{
			float a = param();
			if (a < 0.005f)
				a = 0.005f;
			else if (a > 0.995f)
				a = 0.995f;
			return x > a ? -1.0 : 1.0;
		}
	};


	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Power: public ParametricWave
	{
	  public:
		Core::Sample
		operator() (Core::Sample x, Core::Sample) const
		{
			x = x * 2.0f - 1.0f;
			Core::Sample sgn = x >= 0.0 ? 1.0 : -1.0;
			return sgn * std::pow (mod1 (sgn * x), 50.0 * param() * param() * param());
		}
	};


	/**
	 * Argument: [0, 1.0]. Value for 0.0 == value for 1.0.
	 */
	class Gauss: public ParametricWave
	{
	  public:
		Core::Sample
		operator() (Core::Sample x, Core::Sample) const
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
		Core::Sample
		operator() (Core::Sample x, Core::Sample) const
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
		Core::Sample
		operator() (Core::Sample x, Core::Sample) const
		{
			float a = renormalize (param(), 0.0f, 1.0f, 0.25f, 1.0f);
			x = mod1 (x) * 2.0f * M_PI;
			a = (a - 0.5f) * 4.0f;
			if (a < 0.0f)
				a *= 2.0f;
			a = std::pow (3.0f, a);
			return std::sin (x / 2.0f) * std::sin (a * x * x);
		}
	};

} // namespace ParametricWaves


namespace SeriesFunctions {

/**
 * Argument: [-1.0, 1.0].
 */
template<class Sample>
	inline Sample
	base_sine5 (Sample register x, Sample base_frequency, Sample fmin, Sample fmax)
	{
		if (base_frequency >= fmin)
			return DSP::base_sine5 (x);
		return 0.0;
	}


/**
 * Argument: [-1.0, 1.0].
 */
template<class Sample>
	inline Sample
	base_sine6 (Sample register x, Sample base_frequency, Sample fmin, Sample fmax)
	{
		if (base_frequency >= fmin)
			return DSP::base_sine6 (x);
		return 0.0;
	}


/**
 * Argument: [-1.0, 1.0].
 */
template<class Sample>
	inline Sample
	base_sine7 (Sample register x, Sample base_frequency, Sample fmin, Sample fmax)
	{
		if (base_frequency >= fmin)
			return DSP::base_sine7 (x);
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
			y += sgn * DSP::base_sine5 (z) / pow2<Sample> (2 * k - 1);
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
			y += DSP::base_sine5 (std::fmod ((2 * k - 1) * (x + 1.0f), 2.0f) - 1.0f) / static_cast<Sample> (2 * k - 1);

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
			y += DSP::base_sine5 (std::fmod (k * (x + 1.0f), 2.0f) - 1.0f) / static_cast<Sample> (k);

		return y * M_2_PI;
	}

} // namespace SeriesFunctions

} // namespace DSP

#endif

