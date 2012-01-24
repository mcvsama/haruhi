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

// Standard:
#include <cstddef>
#include <utility>

// Local:
#include "work_performer.h"


WorkPerformer::Performer::Performer (WorkPerformer* work_performer, unsigned int thread_id):
	_work_performer (work_performer),
	_thread_id (thread_id)
{
	// 128k-words stack (512kB on 32-bit, 1MB on 64-bit system) should be sufficient for most operations:
	set_stack_size (128 * sizeof (size_t) * 1024);
}


void
WorkPerformer::Performer::run()
{
	Unit* unit = 0;
	while ((unit = _work_performer->take_unit()))
	{
		unit->_is_ready.store (false);
		unit->_thread_id = _thread_id;
		unit->execute();
		unit->_is_ready.store (true);
		unit->_wait_sem.post();
	}
}


WorkPerformer::WorkPerformer (unsigned int threads_number)
{
	threads_number = std::max (1u, threads_number);

	for (unsigned int i = 0; i < threads_number; ++i)
	{
		Performer* p = new Performer (this, i);
		_performers.push_back (p);
		p->start();
	}
}


WorkPerformer::~WorkPerformer()
{
	for (std::vector<Performer*>::size_type i = 0; i < _performers.size(); ++i)
		_queue_semaphore.post();
	for (std::vector<Performer*>::iterator p = _performers.begin(); p != _performers.end(); ++p)
	{
		(*p)->wait();
		delete *p;
	}
}


void
WorkPerformer::add (Unit* unit)
{
	_queue_mutex.lock();
	unit->added_to_queue();
	_queue.push (unit);
	_queue_mutex.unlock();
	_queue_semaphore.post();
}


void
WorkPerformer::set_sched (Thread::SchedType sched_type, int priority)
{
	for (std::vector<Performer*>::iterator p = _performers.begin(); p != _performers.end(); ++p)
		(*p)->set_sched (sched_type, priority);
}


WorkPerformer::Unit*
WorkPerformer::take_unit()
{
	_queue_semaphore.wait();
	_queue_mutex.lock();

	if (_queue.empty())
	{
		_queue_mutex.unlock();
		return 0;
	}
	else
	{
		Unit* u = _queue.front();
		_queue.pop();
		_queue_mutex.unlock();
		return u;
	}
}

