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

#ifndef HARUHI__UTILITY__WORK_PERFORMER_H__INCLUDED
#define HARUHI__UTILITY__WORK_PERFORMER_H__INCLUDED

// Standard:
#include <cstddef>
#include <queue>
#include <list>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/semaphore.h>
#include <haruhi/utility/thread.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/noncopyable.h>


/**
 * WorkPerformer queues work units (WorkUnit) and executes them in the context
 * of separate thread.
 */
class WorkPerformer: private Noncopyable
{
  private:
	/**
	 * Thread implementation.
	 */
	class Performer: public Thread
	{
	  public:
		Performer (WorkPerformer*, unsigned int thread_id) noexcept;

		void
		run();

	  private:
		WorkPerformer*	_work_performer;
		unsigned int	_thread_id;
	};

  public:
	/**
	 * Implements code that needs to be executed in a separate thread.
	 */
	class Unit: private Noncopyable
	{
		friend class Performer;
		friend class WorkPerformer;

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

		/**
		 * Return true if execute() method is finished.
		 */
		bool
		is_ready() { return _is_ready.load(); }

		/**
		 * Wait for the task to be done.
		 * There can be only one waiting thread.
		 */
		void
		wait() { _wait_sem.wait(); }

		/**
		 * Return thread ID, which is a number between 0 and threads_num-1.
		 * Tells to which executing thread this work unit has been assigned.
		 *
		 * Can be called only after WorkUnit has been started by the Performer
		 * (in and after exit from execute() method).
		 */
		unsigned int
		thread_id() const { return _thread_id; }

	  private:
		/**
		 * Called by the WorkPerformer when unit is added to the queue.
		 */
		void
		added_to_queue();

	  private:
		Atomic<bool>	_is_ready;
		Semaphore		_wait_sem;
		unsigned int	_thread_id;
	};

  private:
	typedef std::queue<Unit*> Units;

	friend class Performer;

  public:
	/**
	 * Create WorkPerformer with given number of threads.
	 * The number of threads never changes.
	 */
	WorkPerformer (unsigned int threads_number);

	/**
	 * Waits for threads to finish before return.
	 */
	~WorkPerformer();

	/**
	 * Add work unit to the queue. The same object may be used
	 * over and over, but not simultaneously.
	 * \threadsafe
	 */
	void
	add (Unit*);

	/**
	 * Set scheduling parameter for all threads.
	 */
	void
	set_sched (Thread::SchedType, int priority) noexcept;

	/**
	 * Return number of threads created.
	 */
	unsigned int
	threads_number() const { return _performers.size(); }

	/**
	 * Unit adaptor.
	 */
	template<class Function>
		static Unit*
		make_unit (Function fun)
		{
			struct Specialized: public Unit
			{
				Specialized (Function fun): _fun (fun) { }

				void execute() { _fun(); }

			  private:
				Function _fun;
			};

			return new Specialized (fun);
		};

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
	std::vector<Performer*>	_performers;
};


inline void
WorkPerformer::Unit::added_to_queue()
{
	_is_ready.store (false);
}

#endif

