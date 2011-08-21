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
 */

#ifndef HARUHI__UTILITY__SEMAPHORE_H__INCLUDED
#define HARUHI__UTILITY__SEMAPHORE_H__INCLUDED

// Standard:
#include <cstddef>

// System:
#include <semaphore.h>


/**
 * OO-oriented semaphore.
 */
class Semaphore
{
  public:
	Semaphore (int value = 0);

	~Semaphore();

	/**
	 * Locks semaphore.
	 */
	void
	wait() const;

	/**
	 * Tries to lock semaphore. Returns true if semaphore was
	 * locked successfully, false otherwise.
	 */
	bool
	try_wait() const;

	/**
	 * Unlocks semaphore.
	 */
	void
	post() const;

  private:
	::sem_t mutable _semaphore;
};

#endif

