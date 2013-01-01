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
 * --
 * The SSE optimized log2f4, exp2f4 and vec4_pow(__m128,__m128) functions were take from José Fonseca's Tech blog.
 * Visit <http://jrfonseca.blogspot.com/2008/09/fast-sse2-pow-tables-or-polynomials.html>
 * for complete code.
 */

#ifndef HARUHI__UTILITY__SSE_POW_H__INCLUDED
#define HARUHI__UTILITY__SSE_POW_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdint.h>

// Haruhi:
#include <haruhi/config/all.h>

#if defined(HARUHI_SSE1) && defined(HARUHI_SSE2)

// System:
#include <xmmintrin.h>

#define HARUHI_HAS_SSE_POW


namespace SSEPow {

/**
 * Initializes SIMD pow function.
 */
extern void
initialize();


/**
 * Deinitializes SIMD pow function.
 */
void
deinitialize();


#define SIMD_POW_EXP2_TABLE_SIZE_LOG2 12
#define SIMD_POW_EXP2_TABLE_SIZE (1 << SIMD_POW_EXP2_TABLE_SIZE_LOG2)
#define SIMD_POW_EXP2_TABLE_OFFSET (SIMD_POW_EXP2_TABLE_SIZE/2)
#define SIMD_POW_EXP2_TABLE_SCALE ((float) ((SIMD_POW_EXP2_TABLE_SIZE/2)-1))

#define SIMD_POW_LOG2_TABLE_SIZE_LOG2 11
#define SIMD_POW_LOG2_TABLE_SIZE (1 << SIMD_POW_LOG2_TABLE_SIZE_LOG2)
#define SIMD_POW_LOG2_TABLE_SCALE ((float) ((SIMD_POW_LOG2_TABLE_SIZE)-1))


union F4 {
	int32_t		i[4];
	uint32_t	u[4];
	float		f[4];
	__m128		m;
	__m128i		mi;
};


/**
 * Fast approximation to exp2(x).
 * Prior to use this function initialize() must be called.
 *
 * Let ipart = int(x)
 * Let fpart = x - ipart;
 * So, exp2(x) = exp2(ipart) * exp2(fpart)
 * Compute exp2(ipart) with i << ipart
 * Compute exp2(fpart) with lookup table.
 */
static inline __m128
exp2f4 (__m128 x) noexcept
{
	extern float exp2_table[2 * SIMD_POW_EXP2_TABLE_SIZE];

	__m128i ipart;
	__m128 fpart, expipart;
	F4 index, expfpart;

	x = _mm_min_ps (x, _mm_set1_ps (+129.00000f));
	x = _mm_max_ps (x, _mm_set1_ps (-126.99999f));

	// ipart = int(x)
	ipart = _mm_cvtps_epi32 (x);

	// fpart = x - ipart
	fpart = _mm_sub_ps (x, _mm_cvtepi32_ps (ipart));

	// expipart = (float) (1 << ipart)
	expipart = _mm_castsi128_ps (_mm_slli_epi32 (_mm_add_epi32 (ipart, _mm_set1_epi32 (127)), 23));

	// index = SIMD_POW_EXP2_TABLE_OFFSET + (int)(fpart * SIMD_POW_EXP2_TABLE_SCALE)
	index.mi = _mm_add_epi32 (_mm_cvtps_epi32 (_mm_mul_ps (fpart, _mm_set1_ps (SIMD_POW_EXP2_TABLE_SCALE))), _mm_set1_epi32 (SIMD_POW_EXP2_TABLE_OFFSET));

	expfpart.f[0] = exp2_table[index.u[0]];
	expfpart.f[1] = exp2_table[index.u[1]];
	expfpart.f[2] = exp2_table[index.u[2]];
	expfpart.f[3] = exp2_table[index.u[3]];

	return _mm_mul_ps (expipart, expfpart.m);
}


/**
 * Fast approximation to log2(x).
 * Prior to use this function initialize() must be called.
 */
static inline __m128
log2f4 (__m128 x) noexcept
{
	extern float log2_table[2 * SIMD_POW_LOG2_TABLE_SIZE];

	F4 index, p;

	__m128i exp = _mm_set1_epi32 (0x7F800000);
	__m128i mant = _mm_set1_epi32 (0x007FFFFF);

	__m128i i = _mm_castps_si128 (x);

	__m128 e = _mm_cvtepi32_ps (_mm_sub_epi32 (_mm_srli_epi32 (_mm_and_si128 (i, exp), 23), _mm_set1_epi32 (127)));

	index.mi = _mm_srli_epi32 (_mm_and_si128 (i, mant), 23 - SIMD_POW_LOG2_TABLE_SIZE_LOG2);

	p.f[0] = log2_table[index.u[0]];
	p.f[1] = log2_table[index.u[1]];
	p.f[2] = log2_table[index.u[2]];
	p.f[3] = log2_table[index.u[3]];

	return _mm_add_ps (p.m, e);
}


/**
 * Returns x^y.
 * Prior to use this function initialize() must be called.
 */
static inline __m128
vec4_pow (__m128 radix, __m128 power) noexcept
{
	return exp2f4 (_mm_mul_ps (log2f4 (radix), power));
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline __m128
vec4_pow_radix_2 (__m128 power) noexcept
{
	return exp2f4 (power);
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline __m128
vec4_exp (__m128 power) noexcept
{
	return exp2f4 (_mm_mul_ps (_mm_set_ps1 (1.44269504088896f), power));
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline __m128
vec4_radix_10 (__m128 power) noexcept
{
	return exp2f4 (_mm_mul_ps (_mm_set_ps1 (3.32192809488736f), power));
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline float
pow (float radix, float power) noexcept
{
	float result;
	_mm_store_ss (&result, vec4_pow (_mm_set_ps1 (radix), _mm_set_ps1 (power)));
	return result;
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline float
pow_radix_2 (float power) noexcept
{
	float result;
	_mm_store_ss (&result, exp2f4 (_mm_set_ps1 (power)));
	return result;
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline float
exp (float power) noexcept
{
	float result;
	_mm_store_ss (&result, exp2f4 (_mm_mul_ps (_mm_set_ps1 (1.44269504088896f), _mm_set_ps1 (power))));
	return result;
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline float
pow_radix_10 (float power) noexcept
{
	float result;
	_mm_store_ss (&result, exp2f4 (_mm_mul_ps (_mm_set_ps1 (3.32192809488736f), _mm_set_ps1 (power))));
	return result;
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline void
vec4_pow (float* result, float* radix, float* power) noexcept
{
	_mm_store_ps (result, vec4_pow (_mm_load_ps (radix), _mm_load_ps (power)));
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline void
vec4_pow_to_scalar (float* result, float* radix, float power) noexcept
{
	_mm_store_ps (result, vec4_pow (_mm_load_ps (radix), _mm_set_ps1 (power)));
}

} // namespace SSEPow

#endif

#endif

