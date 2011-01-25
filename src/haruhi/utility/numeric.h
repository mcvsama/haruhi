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

#ifndef HARUHI__UTILITY__NUMERIC_H__INCLUDED
#define HARUHI__UTILITY__NUMERIC_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <stdint.h>


inline float
renormalize (float value, float a1, float b1, float a2, float b2)
{
	if (b1 == a1)
		return a2;
	float scaler = (b2 - a2) / (b1 - a1);
	float offset = -scaler * a1 + a2;
	return scaler * value + offset;
}


inline int
cut (int value)
{
	return (value > 255)? 255 : value;
}


template<class Value>
	inline Value
	bound (Value value, Value min, Value max)
	{
		return (value < min)
			? min
			: (value > max)
				? max
				: value;
	}


template<class Value>
	inline void
	limit_value (Value& value, Value min, Value max)
	{
		value = bound (value, min, max);
	}


template<class Value>
	inline Value
	pow2 (Value value)
	{
		return value * value;
	}


/**
 * Returns fractional part of float.
 * For IEEE-754 only.
 */
inline float
ieee754_fractional_part (const float& v)
{
	union {
		float f;
		uint32_t i;
	} u = {v};
	// Sign:
	const uint32_t s = u.i & (1 << 31);
	// Get exponent:
	const int32_t e = (((0x7F800000 & u.i) >> 23) - 127);
	// Get only the digits of the mantissa that will result in a fraction:
	const uint32_t m = ((0x007FFFFF & u.i)) & (0x007FFFFF >> e);
	// Less than 1, so just return as the next part requires e >= 1:
	if (e < 0)
		return v;
	// Check for no fractional part, the check on m can be removed
	// but rounding errors will result in very small values for powers of 2.
	if (e > 23 || m == 0)
		return 0.0;
	// Insert favourite integer to float cast here:
	u.f = static_cast<float> (m);
	// Keep new mantissa, resize exponent:
	u.i = (u.i & ~0x7F800000) | ((e + ((u.i & 0x7F800000) >> 23) - 23) << 23) | s;
	return u.f;
}


/**
 * Returns fractional part of float.
 * Fast when compiler uses SSE3's fisttp instruction.
 */
inline float
sse3_fractional_part (const float& v)
{
	union {
		float f;
		uint32_t i;
	} u = {v};
	// Sign:
	const uint32_t s = u.i & (1 << 31);
	u.f = u.f - static_cast<int> (u.f);
	u.i |= s;
	return u.f;
}


/**
 * Returns x > 0.0 ? 1.0 : -1.0.
 */
inline float
sgn (float const& v)
{
	union {
		float f;
		uint32_t i;
	} u = {v};
	const uint32_t s = u.i & (1 << 31);
	u.f = 1.0f;
	u.i |= s;
	return u.f;
}


/**
 * Fmods value x into [0, 1.0] range.
 */
inline float
mod1 (float x)
{
#if defined HARUHI_SSE3
	return x >= 0.0 ? sse3_fractional_part (x) : 1.0f - sse3_fractional_part (-x);
#elif defined HARUHI_IEEE754
	return x >= 0.0 ? ieee754_fractional_part (x) : 1.0f - ieee754_fractional_part (-x);
#else
	return x >= 0.0 ? std::fmod (x, 1.0f) : 1.0f - std::fmod (-x, 1.0f);
#endif
}


/**
 * Returns attenuation factor from given number of decibels.
 * \param	db Attenuation value (-inf..0), eg. -3 means -3dB.
 */
inline float
attenuate_db (float db)
{
	return powf (10.0, db / 10.0);
}

#endif

