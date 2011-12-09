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

#ifndef HARUHI__UTILITY__THREAD_H__INCLUDED
#define HARUHI__UTILITY__THREAD_H__INCLUDED

// Standard:
#include <cstddef>

// System:
#include <pthread.h>

// Haruhi:
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/noncopyable.h>


/**
 * Thread is created in "detached" state, that means it will
 * automatically free its resources after exit.
 */
class Thread: private Noncopyable
{
  public:
	enum {
		MinimumStackSize = PTHREAD_STACK_MIN
	};

	enum SchedType {
		SchedFIFO	= SCHED_FIFO,
		SchedRR		= SCHED_RR,
		SchedOther	= SCHED_OTHER
	};

	typedef pthread_t ID;

  public:
	Thread();

	/**
	 * Waits for thread to exit.
	 * Be sure to call wait() in destructor in derived
	 * class to be sure that derived object is not destructed before thread exits.
	 */
	virtual ~Thread();

	/**
	 * Starts thread.
	 */
	virtual void
	start();

	/**
	 * Cancels thread.
	 */
	virtual void
	cancel();

	/**
	 * \returns	true if thread has finished.
	 */
	virtual bool
	finished();

	/**
	 * Set scheduling policy/parameter for thread.
	 */
	virtual void
	set_sched (SchedType, int priority);

	/**
	 * Sets stack size for new thread.
	 * If 0, system default will be used.
	 */
	virtual void
	set_stack_size (std::size_t size);

	/**
	 * Waits for thread to finish.
	 * May be called by many other threads, but MUST NOT
	 * be called from within this thread.
	 */
	virtual void
	wait();

	/**
	 * Forces calling thread to relinquish use of its processor,
	 * and to wait in the run queue.
	 */
	static void
	yield();

	static ID
	id();

  protected:
	virtual void
	run() = 0;

  private:
	void
	set_sched();

	static void*
	callback (void *arg);

	pthread_t		_pthread;
	SchedType		_sched_type;
	int				_priority;
	std::size_t		_stack_size;
	Atomic<bool>	_started;
	Atomic<bool>	_finished;
	Mutex			_wait;
};

#endif

