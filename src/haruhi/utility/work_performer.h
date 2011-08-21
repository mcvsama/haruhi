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

#ifndef HARUHI__UTILITY__WORK_PERFORMER_H__INCLUDED
#define HARUHI__UTILITY__WORK_PERFORMER_H__INCLUDED

// Standard:
#include <cstddef>
#include <queue>
#include <list>

// Haruhi:
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/semaphore.h>
#include <haruhi/utility/thread.h>
#include <haruhi/utility/atomic.h>


/**
 * WorkPerformer queues work units (WorkUnit) and executes them in the context
 * of the current thread.
 */
class WorkPerformer
{
  private:
	/**
	 * Thread implementation.
	 */
	class Performer: public Thread
	{
	  public:
		Performer (WorkPerformer*);

		void
		run();

	  private:
		WorkPerformer* _work_performer;
	};

  public:
	/**
	 * Implements code that needs to be executed in a separate thread.
	 */
	class Unit
	{
		friend class Performer;

	  public:
		/**
		 * Deleted by WorkPerformer.
		 */
		virtual ~Unit() { }

		/**
		 * Starts work.
		 */
		virtual void
		execute() = 0;

		bool
		is_ready() { return _is_ready.load(); }

		/**
		 * Wait for the task to be done.
		 * There can be only one waiting thread.
		 */
		void
		wait() { _wait_sem.wait(); }

	  private:
		Atomic<bool>	_is_ready;
		Semaphore		_wait_sem;
	};

  private:
	typedef std::queue<Unit*> Units;

	friend class Performer;

  public:
	WorkPerformer (unsigned int threads_number);

	/**
	 * Waits for threads to finish.
	 * Posts dummy work units.
	 */
	~WorkPerformer();

	/**
	 * Add work unit to the queue.
	 * Object pushed to the queue will be deleted with delete operator in the context
	 * of executing thread (method execute()).
	 * \threadsafe
	 */
	void
	add (Unit*);

  private:
	/**
	 * Take unit from the queue. If there are no units ready, wait
	 * until new unit arrives. Return 0 if thread should exit.
	 * \threadsafe
	 */
	Unit*
	take_unit();

  private:
	// Current queue. Points either to _queues[1] or _queues[2]:
	Units					_queue;
	Mutex					_queue_mutex;
	Semaphore				_queue_semaphore;
	std::list<Performer*>	_performers;
};

#endif

