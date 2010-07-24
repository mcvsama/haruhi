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

#ifndef HARUHI__CONFIG__SYSTEM_H__INCLUDED
#define HARUHI__CONFIG__SYSTEM_H__INCLUDED

#undef assert

#if HARUHI_ENABLE_ASSERT

#include <signal.h>

#undef assert

inline void
assert (bool expression)
{
	if (!expression)
	{
#if HARUHI_ENABLE_FATAL_ASSERT
		raise (SIGTRAP);
#endif
	}
}

#else // HARUHI_ENABLE_ASSERT

#undef assert
#define assert

#endif // HARUHI_ENABLE_ASSERT

#endif

