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
 * Here be basic, global functions and macros like asserts, debugging helpers, etc.
 */

#ifndef HARUHI__CONFIG__SYSTEM_H__INCLUDED
#define HARUHI__CONFIG__SYSTEM_H__INCLUDED

// Standard:
#include <cstdio>
#include <iostream>

#ifdef __SSE__
#define HARUHI_SSE1
#endif

#ifdef __SSE2__
#define HARUHI_SSE2
#endif

#ifdef __SSE3__
#define HARUHI_SSE3
#endif


// Fixes for std::ostream which has broken support for unsigned/signed/char types
// and prints 8-bit integers like they were characters.
namespace ostream_fixes {

inline std::ostream&
operator<< (std::ostream& os, unsigned char i)
{
	return os << static_cast<unsigned int> (i);
}


inline std::ostream&
operator<< (std::ostream& os, signed char i)
{
	return os << static_cast<signed int> (i);
}

} // namespace ostream_fixes


using namespace ostream_fixes;

namespace std {

// Missing in GCC 7.2
// TODO remove when GCC is fixed.
static constexpr std::size_t hardware_destructive_interference_size = 64;

} // namespace std

inline void
assert_function (bool expression, const char* message = nullptr) noexcept
{
	if (!expression)
	{
		if (message)
			std::clog << "Assertion failed: " << message << std::endl;
# if HARUHI_ENABLE_FATAL_ASSERT
		raise (SIGTRAP);
# endif
	}
}


/**
 * Prints debug output.
 */
#define xdebug(...) fprintf (stderr, ...)

#endif


/**
 * Since most of standard headers override our assert, ensure
 * that it's redefined every possible time, when this
 * header is included.
 */

#undef assert
#if HARUHI_ENABLE_ASSERT
# include <signal.h>
# undef assert
# define assert assert_function
#else // HARUHI_ENABLE_ASSERT
# undef assert
# define assert(a)
#endif // HARUHI_ENABLE_ASSERT

