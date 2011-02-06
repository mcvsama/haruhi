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
 * --
 * The SSE optimized log2f4, exp2f4 and powf4 functions were take from José Fonseca's Tech blog.
 * Visit <http://jrfonseca.blogspot.com/2008/09/fast-sse2-pow-tables-or-polynomials.html>
 * for complete code.
 */

// Standard:
#include <cstddef>
#include <cmath>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "sse_pow.h"


namespace SSEPow {

// 2 ^ x, for x in [-1.0, 1.0[
float exp2_table[2 * SIMD_POW_EXP2_TABLE_SIZE];
// log2(x), for x in [1.0, 2.0[
float log2_table[2 * SIMD_POW_LOG2_TABLE_SIZE];


void exp2_init()
{
   int i;
   for (i = 0; i < SIMD_POW_EXP2_TABLE_SIZE; i++)
      exp2_table[i] = ::pow (2.0, (i - SIMD_POW_EXP2_TABLE_OFFSET) / SIMD_POW_EXP2_TABLE_SCALE);
}


void log2_init()
{
	unsigned i;
	for (i = 0; i < SIMD_POW_LOG2_TABLE_SIZE; i++)
		log2_table[i] = ::log2 (1.0 + i * (1.0 / (SIMD_POW_LOG2_TABLE_SIZE-1)));
}


void
initialize()
{
	exp2_init();
	log2_init();
}


void
deinitialize()
{
}

} // namespace SSEPow

