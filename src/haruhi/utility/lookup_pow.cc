/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 * --
 * Implementation of the pow_generic() and initialize() functions were
 * obtained from <http://www.hxa.name/articles/content/fast-pow-adjustable_hxa7241_2007.html>
 * and are distributed under following license:
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

// Standard:
#include <cstddef>
#include <cmath>

// Local:
#include "fast_pow.h"


namespace LookupPow {

unsigned int* lookup_table;


/**
 * Initializes lookup table with given precision.
 *
 * \param	pTable Length must be 2 ^ precision
 * \param	precision Number of mantissa bits used, >= 0 and <= 18
 */
static void
generic_initialize (unsigned int* const pTable, const unsigned int precision) noexcept
{
	// step along table elements and x-axis positions
	float zeroToOne = 1.0f / (static_cast<float> (1 << precision) * 2.0f);

	for (int i = 0; i < (1 << precision); ++i)
	{
		// make y-axis value for table element
		const float f = (static_cast<float> (std::pow (2.0f, zeroToOne)) - 1.0f) * fast_pow_2p23;
		pTable[i] = static_cast<unsigned int> (f < fast_pow_2p23 ? f : (fast_pow_2p23 - 1.0f));

		zeroToOne += 1.0f / static_cast<float> (1 << precision);
	}
}


void
initialize()
{
	lookup_table = new unsigned int[1 << FastPowLookupPrecision];
	generic_initialize (lookup_table, FastPowLookupPrecision);
}


void
deinitialize()
{
	delete[] lookup_table;
}

} // namespace LookupPow

