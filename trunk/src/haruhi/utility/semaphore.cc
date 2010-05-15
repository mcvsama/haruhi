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

// Standard:
#include <cstddef>

// System:
#include <semaphore.h>
#include <errno.h>

// Local:
#include "semaphore.h"


Semaphore::Semaphore (int value)
{
	::sem_init (&_semaphore, 0, value);
}


Semaphore::~Semaphore()
{
	::sem_destroy (&_semaphore);
}


void
Semaphore::wait() const
{
	::sem_wait (&_semaphore);
}


bool
Semaphore::try_wait() const
{
	if (::sem_trywait (&_semaphore) == -1)
		return false;
	return true;
}


void
Semaphore::post() const
{
	::sem_post (&_semaphore);
}

