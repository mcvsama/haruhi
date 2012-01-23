/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 * --
 * Implementation of the pow_generic() and initialize() functions were
 * obtained from <http://www.hxa.name/articles/content/fast-pow-adjustable_hxa7241_2007.html>
 *
 * Small fix was added to fast_pow_generic(): prevents producing insanely large values or nans
 * when used on small radices with large powers (say 0.00001^15.0).
 * --
 * This code is distributed under following license:
 *
 * Copyright (c) 2007, Harrison Ainsworth / HXA7241.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * • Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * • Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * • The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HARUHI__UTILITY__LOOKUP_POW_H__INCLUDED
#define HARUHI__UTILITY__LOOKUP_POW_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <stdint.h>

// Haruhi:
#include <haruhi/config/all.h>


namespace LookupPow {

const int FastPowLookupPrecision = 16;
const float fast_pow_2p23 = 8388608.0f;


/**
 * Initializes lookup table for fast_pow function.
 */
extern void
initialize();


/**
 * Deinitializes fast_pow_lookup table.
 */
extern void
deinitialize();


/**
 * Prior to use this function initialize() must be called.
 *
 * \param	power Power to raise radix to
 * \param	ilog2 One over log, to required radix, of two
 */
static inline float
pow_generic (const float power, const float ilog2) noexcept
{
	extern unsigned int* lookup_table;

	// build float bits
	int i = static_cast<int> ((power * (fast_pow_2p23 * ilog2)) + (127.0f * fast_pow_2p23));

	// Fix for small radix and large power:
	if (i < 0)
		i = 0;

	// replace mantissa with lookup
	const int it = (i & 0xFF800000) | lookup_table[(i & 0x7FFFFF) >> (23 - FastPowLookupPrecision)];

	// convert bits to float
	union { float f; int i; } u = { i: it };
	return u.f;
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline float
pow (const float radix, const float power) noexcept
{
	return pow_generic (power, std::log (radix) * 1.44269504088896f);
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline float
pow_radix_2 (const float power) noexcept
{
	return pow_generic (power, 1.0f);
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline float
exp (const float power) noexcept
{
	return pow_generic (power, 1.44269504088896f);
}


/**
 * Prior to use this function initialize() must be called.
 */
static inline float
pow_radix_10 (const float power) noexcept
{
	return pow_generic (power, 3.32192809488736f);
}

} // namespace LookupPow

#endif

