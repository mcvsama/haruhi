/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 * --
 * Here be basic, global functions and macros like asserts, debugging helpers, etc.
 */

#ifndef HARUHI__CONFIG__SYSTEM_H__INCLUDED
#define HARUHI__CONFIG__SYSTEM_H__INCLUDED

#include <cstdio>

#ifdef __SSE__
#define HARUHI_SSE1
#endif

#ifdef __SSE2__
#define HARUHI_SSE2
#endif

#ifdef __SSE3__
#define HARUHI_SSE3
#endif

#undef assert

#if HARUHI_ENABLE_ASSERT
# include <signal.h>
# undef assert

inline void
assert (bool expression)
{
	if (!expression)
	{
# if HARUHI_ENABLE_FATAL_ASSERT
		raise (SIGTRAP);
# endif
	}
}

#else // HARUHI_ENABLE_ASSERT
# undef assert
# define assert
#endif // HARUHI_ENABLE_ASSERT


/**
 * Return size (number of elements) of an array.
 */
template<class T, std::size_t N>
	inline std::size_t
	countof (T(&)[N])
	{
		return N;
	}


/**
 * Return size of an array. Can be used in const expressions.
 */
template<class T, std::size_t N>
	inline const char (&sizer (T (&)[N]))[N];


/**
 * Prints debug output.
 */
#define debug(x...) fprintf (stderr, x)

#endif

