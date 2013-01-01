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

// Standard:
#include <cstddef>
#include <stdexcept>
#include <inttypes.h>

// System:
#include <pthread.h>
#include <errno.h>

// Local:
#include "condition.h"


Condition::Condition() noexcept
{
	static ::pthread_mutex_t mutex_initializer = PTHREAD_MUTEX_INITIALIZER;
	static ::pthread_cond_t cond_initializer = PTHREAD_COND_INITIALIZER;

	_mutex = mutex_initializer;
	_cond = cond_initializer;
}


Condition::~Condition()
{
	this->unlock();
	::pthread_cond_destroy (&_cond);
	::pthread_mutex_destroy (&_mutex);
}


void
Condition::lock() noexcept
{
	::pthread_mutex_lock (&_mutex);
}


void
Condition::unlock()
{
	switch (::pthread_mutex_unlock (&_mutex))
	{
		case EPERM:
			throw ConditionPermissionException ("the calling thread does not own the condition variable", __func__);
	}
}


void
Condition::wait (uint64_t nanoseconds) noexcept
{
	if (nanoseconds)
	{
		struct timespec t;
		t.tv_sec = nanoseconds / 1000000000;
		t.tv_nsec = nanoseconds % 1000000000;
		::pthread_cond_timedwait (&_cond, &_mutex, &t);
	}
	else
	{
		::pthread_cond_wait (&_cond, &_mutex);
	}
}


void
Condition::signal() noexcept
{
	::pthread_cond_signal (&_cond);
}

