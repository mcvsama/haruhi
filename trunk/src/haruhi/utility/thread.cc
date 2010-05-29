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
#include <stdexcept>
#include <cstring>

// System:
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

// Local:
#include <haruhi/utility/atomic.h>

#include "thread.h"


Thread::Thread():
	_sched_type (SchedOther),
	_priority (50),
	_stack_size (0),
	_started (false),
	_finished (false)
{
}


Thread::~Thread()
{
	wait();
}


void
Thread::start()
{
	pthread_attr_t att;
	pthread_attr_init (&att);
	// Scheduling policy, doesn't work anyway.
	// Will set_sched inside of callback(), before run().
	pthread_attr_setschedpolicy (&att, _sched_type);
	// Scheduling param:
	struct sched_param p;
	memset (&p, 0, sizeof p);
	p.sched_priority = _priority;
	pthread_attr_setschedparam (&att, &p);
	pthread_attr_setdetachstate (&att, PTHREAD_CREATE_DETACHED);
	if (_stack_size)
	{
		const std::size_t page_size = sysconf (_SC_PAGESIZE);
		pthread_attr_setstacksize (&att, ((_stack_size / page_size) + 1) * page_size);
	}
	// Start thread:
	switch (::pthread_create (&_pthread, &att, callback, this))
	{
		case EAGAIN:
			throw std::runtime_error ("not enough system resources or maximum Threads count achieved");
	}
	pthread_attr_destroy (&att);
}

void
Thread::cancel()
{
	::pthread_cancel (_pthread);
}


bool
Thread::finished()
{
	return atomic (_finished);
}


void
Thread::set_sched (SchedType type, int priority)
{
	_sched_type = type;
	_priority = priority;
	set_sched();
}


void
Thread::set_stack_size (std::size_t size)
{
	_stack_size = size;
}


void
Thread::yield()
{
	::pthread_yield();
}


Thread::ID
Thread::id()
{
	return ::pthread_self();
}


void
Thread::wait()
{
	_wait.lock();
	_wait.unlock();
}


void
Thread::set_sched()
{
	if (_started && !_finished)
	{
		struct sched_param p;
		p.sched_priority = _priority;
		::pthread_setschedparam (_pthread, _sched_type, &p);
	}
}


void*
Thread::callback (void* arg)
{
	Thread *k = reinterpret_cast<Thread*> (arg);
	atomic (k->_started) = true;
	k->set_sched();
	k->_wait.lock();
	atomic (k->_finished) = false;
	k->run();
	atomic (k->_finished) = true;
	k->_wait.unlock();
	return 0;
}

