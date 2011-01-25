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
 * Here be basic, global functions and macros like asserts, debugging helpers, etc.
 */

#ifndef HARUHI__CONFIG__SYSTEM_H__INCLUDED
#define HARUHI__CONFIG__SYSTEM_H__INCLUDED

#include <cstdio>


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
 * Returns size (number of elements) of an array.
 */
#define ARRAY_SIZE(x) (sizeof (x) / sizeof (*x))


/**
 * Statically asserts that x is true. If it's not, there will
 * be error at compile time.
 * \param	msg must be valid C++ identifier.
 */
#define static_assert(x, msg)								\
	do {													\
		struct StaticAssertFailure_##msg { };				\
		typedef StaticAssert::Check<(x) != 0> Check;		\
		Check check = Check (StaticAssertFailure_##msg());	\
		check.f();											\
	} while (0);

namespace StaticAssert {
	template<bool>	struct Check		{ void f() { }; Check (...) { }; };
	template<>		struct Check<false>	{ void f() { }};
} // namespace StaticAssert


/**
 * Prints debug output.
 */
#define debug(x...) fprintf (stderr, x)

#endif

