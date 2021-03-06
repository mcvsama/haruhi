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

#ifndef HARUHI__UTILITY__NUMERIC_H__INCLUDED
#define HARUHI__UTILITY__NUMERIC_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <stdint.h>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/range.h>

// System:
#ifdef HARUHI_SSE2
#include <xmmintrin.h>
#endif


constexpr float
renormalize (float value, float a1, float b1, float a2, float b2) noexcept
{
	return b1 == a1
		? a2
		: (b2 - a2) / (b1 - a1) * value + (-(b2 - a2) / (b1 - a1) * a1 + a2);
}


constexpr float
renormalize (float value, Range<float> range1, Range<float> range2) noexcept
{
	return renormalize (value, range1.min(), range1.max(), range2.min(), range2.max());
}


constexpr int
cut (int value) noexcept
{
	return (value > 255)
		? 255
		: value;
}


inline void
undenormalize (float& sample) noexcept
{
	union { float f; unsigned int i; } u;
	u.f = sample;
	if ((u.i & 0x7f800000) == 0)
		sample = 0.0f;
}


template<class Value>
	constexpr Value
	ifnan (Value const& test, Value val) noexcept
	{
		return std::isnan (test)
			? val
			: test;
	}


template<class Value>
	constexpr void
	clamp (Value& value, Value min, Value max) noexcept
	{
		if (value < min)
			value = min;
		else if (value > max)
			value = max;
	}


template<class Value>
	constexpr void
	clamp (Value& value, Range<Value> range) noexcept
	{
		clamp (value, range.min(), range.max());
	}


template<class Value>
	constexpr Value
	clamped (Value value, Value min, Value max) noexcept
	{
		return value < min
			? min
			: value > max
				? max
				: value;
	}


template<class Value>
	constexpr Value
	clamped (Value value, Range<Value> range) noexcept
	{
		return clamped (value, range.min(), range.max());
	}


template<class Value>
	constexpr Value
	pow2 (Value v) noexcept
	{
		return v * v;
	}


/**
 * Returns fractional part of float.
 * For IEEE-754 only.
 */
inline float
ieee754_fractional_part (const float& v) noexcept
{
	union { float f; uint32_t i; } u;
	u.f = v;
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
sse3_fractional_part (const float& v) noexcept
{
	union {
		float f;
		uint32_t i;
	} u;
	u.f = v;
	// Sign:
	const uint32_t s = u.i & (1 << 31);
	u.f = u.f - static_cast<int> (u.f);
	u.i |= s;
	return u.f;
}


inline float
fractional_part (const float& v) noexcept
{
#if defined HARUHI_SSE3
	return sse3_fractional_part (v);
#elif defined HARUHI_IEEE754
	return ieee754_fractional_part (v);
#else
	return std::fmod (v, 1.0f);
#endif
}


#ifdef HARUHI_SSE2
/**
 * SIMD version of fractional_part().
 */
inline __m128
fractional_part (__m128 x) noexcept
{
    __m128i a = _mm_cvttps_epi32 (x);
    __m128 b = _mm_cvtepi32_ps (a);
    return _mm_sub_ps (x, b);
}
#endif


/**
 * Returns x > 0.0 ? 1.0 : -1.0.
 */
inline float
sgn (float const& v) noexcept
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
mod1 (float v) noexcept
{
	return v >= 0.0f ? fractional_part (v) : 1.0f - fractional_part (-v);
}


/**
 * Returns attenuation factor from given number of decibels.
 * \param	db Attenuation value (-inf..0), eg. -3 means -3dB.
 */
constexpr float
attenuate_db (float db) noexcept
{
	return std::pow (10.0f, db / 20.0f);
}


/**
 * Stop condition function for get_offset_array(). See below.
 */
template<class T, std::size_t N, std::size_t I>
	constexpr std::array<T, N>
	get_offset_array (std::array<T, N> input, T offset, std::false_type)
	{
		std::get<I> (input) += offset;

		return input;
	}


/**
 * Recursive modifier of array for get_offset_array(). See below.
 */
template<class T, std::size_t N, std::size_t I>
	constexpr std::array<T, N>
	get_offset_array (std::array<T, N> input, T offset, std::true_type)
	{
		std::get<I> (input) += offset;

		return get_offset_array<T, N, I - 1> (input, offset, std::integral_constant<bool, I >= 2>());
	}


/**
 * Return new array with all values modified by adding the offset argument.
 */
template<class T, std::size_t N>
	constexpr std::array<T, N>
	get_offset_array (std::array<T, N> input, T offset)
	{
		return get_offset_array<T, N, N - 1> (input, offset, std::integral_constant<bool, N - 1 >= 2>());
	}

#endif

