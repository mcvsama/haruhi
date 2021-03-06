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
#include <haruhi/utility/literals.h>


/**
 * ConditionPermissionException
 */
class ConditionPermissionException: public std::runtime_error
{
  public:
	explicit
	ConditionPermissionException (const char* what, const char* details):
		std::runtime_error ((what + ": "_str + details).c_str())
	{ }
};


/**
 * OO-oriented condition variable.
 */
class Condition: private Noncopyable
{
  public:
	Condition() noexcept;

	~Condition();

	/**
	 * Locks condition variable.
	 */
	void
	lock() noexcept;

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
	wait (uint64_t nanoseconds = 0) noexcept;

	/**
	 * Signals condition. Should be called when condition variable
	 * is locked.
	 */
	void
	signal() noexcept;

  private:
	::pthread_mutex_t	_mutex;
	::pthread_cond_t	_cond;
};

#endif

