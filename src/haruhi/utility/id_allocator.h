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

#ifndef HARUHI__UTILITY__ID_ALLOCATOR_H__INCLUDED
#define HARUHI__UTILITY__ID_ALLOCATOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Haruhi:
#include <haruhi/config/all.h>


class IDAllocator
{
  public:
	typedef std::set<unsigned int>  IDs;

  public:
	/**
	 * Allocates new ID.
	 */
	unsigned int
	allocate_id();

	/**
	 * Reserves given ID. If not possible (already reserved)
	 * returns new ID.
	 */
	unsigned int
	reserve_id (unsigned int id);

	/**
	 * Frees given ID.
	 */
	void
	free_id (unsigned int id);

	/**
	 * Remove all IDs.
	 */
	void
	clear();

  private:
	IDs	_ids;
};

#endif

