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

#ifndef HARUHI__UTILITY__ATOMIC_H__INCLUDED
#define HARUHI__UTILITY__ATOMIC_H__INCLUDED

// Standard:
#include <cstddef>
#include <atomic>

// Haruhi:
#include <haruhi/config/all.h>


template<class T>
	using Atomic = std::atomic<T>;

#endif

