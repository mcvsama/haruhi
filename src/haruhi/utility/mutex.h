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

#ifndef HARUHI__UTILITY__MUTEX_H__INCLUDED
#define HARUHI__UTILITY__MUTEX_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>

// System:
#include <pthread.h>
#include <errno.h>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/noncopyable.h>


/**
 * MutexPermissionException
 */
class MutexPermissionException: public std::runtime_error
{
  public:
	explicit MutexPermissionException (const char* what, const char* details):
		std::runtime_error ((std::string (what) + ": " + details).c_str())
	{ }
};


/**
 * OO-oriented mutex.
 */
class Mutex: private Noncopyable
{
  public:
	/**
	 * Normal mutexes can be locked only once.
	 *
	 * Recursive mutexec can be locked many times
	 * by the same thread (and then they must be
	 * unlocked the same number of times).
	 */
	enum MutexType { Normal, Recursive };

  public:
	/**
	 * \param	mutex_kind
	 * 			Type of mutex (Normal or Recursive).
	 */
	Mutex (MutexType = Normal) noexcept;

	~Mutex() noexcept;

	/**
	 * Locks mutex or waits to be released and locks.
	 */
	void
	lock() const noexcept;

	/**
	 * Tries to lock mutex. Returns true if mutex was
	 * locked successfully, and false if it was not.
	 */
	bool
	try_lock() const noexcept;

	/**
	 * Unlocks mutex. If calling thread is not the thread
	 * that locked mutex, std::invalid_argument is thrown.
	 *
	 * \throws	std::invalid_argument
	 * 			When calling thread does not own the mutex.
	 */
	void
	unlock() const;

	/**
	 * Unlocks and locks mutex again.
	 */
	void
	yield() const;

	/**
	 * Lock, execute function, and unlock.
	 */
	template<class Callback>
		void
		synchronize (Callback function) const noexcept (noexcept (function()));

	/**
	 * Helper for lock-safe copying some value (eg. for returning):
	 * Like: return graph()->safe_copy (_some_value);
	 */
	template<class Type>
		Type
		safe_copy (Type const& value) const noexcept (noexcept (Type::operator= (value)));

	/**
	 * Helper for unlocking and returning given value.
	 */
	template<class Type>
		Type const&
		unlock_and_return (Type const& value) const;

	/**
	 * Helper for unlocking and throwing given object.
	 */
	template<class Type>
		void
		unlock_and_throw (Type const& value) const;

  private:
	::pthread_mutex_t mutable _mutex;
};


class RecursiveMutex: public Mutex
{
  public:
	RecursiveMutex() noexcept;
};


inline void
Mutex::lock() const noexcept
{
	::pthread_mutex_lock (&_mutex);
}


inline bool
Mutex::try_lock() const noexcept
{
	switch (::pthread_mutex_trylock (&_mutex))
	{
		case EBUSY:
			return false;

		case 0:
			return true;
	}
	return false;
}


inline void
Mutex::unlock() const
{
	switch (::pthread_mutex_unlock (&_mutex))
	{
		case EPERM:
			throw MutexPermissionException ("the calling thread does not own the mutex", __func__);
	}
}


inline void
Mutex::yield() const
{
	unlock();
	lock();
}


template<class Callback>
	inline void
	Mutex::synchronize (Callback function) const noexcept (noexcept (function()))
	{
		lock();
		if (noexcept (function()))
		{
			function();
			unlock();
		}
		else
		{
			try {
				function();
				unlock();
			}
			catch (...)
			{
				unlock();
				throw;
			}
		}
	}


template<class Type>
	inline Type
	Mutex::safe_copy (Type const& value) const noexcept (noexcept (Type::operator= (value)))
	{
		lock();
		Type copy = value;
		unlock();
		return copy;
	}


template<class Type>
	inline Type const&
	Mutex::unlock_and_return (Type const& value) const
	{
		unlock();
		return value;
	}


template<class Type>
	inline void
	Mutex::unlock_and_throw (Type const& value) const
	{
		unlock();
		throw value;
	}

#endif

