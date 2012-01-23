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
 */

#ifndef HARUHI__UTILITY__CONDITION_H__INCLUDED
#define HARUHI__UTILITY__CONDITION_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>
#include <inttypes.h>

// System:
#include <pthread.h>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/noncopyable.h>


/**
 * ConditionPermissionException
 */
class ConditionPermissionException: public std::runtime_error
{
  public:
	explicit ConditionPermissionException (const char* what, const char* details):
		std::runtime_error ((std::string (what) + ": " + details).c_str())
	{ }
};


/**
 * OO-oriented condition variable.
 */
class Condition: private Noncopyable
{
  public:
	Condition();

	~Condition();

	/**
	 * Locks condition variable.
	 */
	void
	lock();

	/**
	 * Unlocks condition variable.
	 */
	void
	unlock();

	/**
	 * Block calling thread to wait on condition. Should be called
	 * when condition variable is locked.
	 */
	void
	wait (uint64_t nanoseconds = 0);

	/**
	 * Signals condition. Should be called when condition variable
	 * is locked.
	 */
	void
	signal();

  private:
	::pthread_mutex_t	_mutex;
	::pthread_cond_t	_cond;
};

#endif

