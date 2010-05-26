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
#include <set>
#include <iostream>//XXX

// Qt:
#include <QtCore/QTimer>

// Haruhi:
#include <haruhi/exception.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "periodic_updater.h"


PeriodicUpdater* PeriodicUpdater::_singleton = 0;


void
PeriodicUpdater::Receiver::schedule_for_update (Thread thread)
{
	PeriodicUpdater::singleton()->schedule (this, thread);
}


void
PeriodicUpdater::Receiver::forget_about_update (Thread thread)
{
	PeriodicUpdater::singleton()->forget (this, thread);
}


PeriodicUpdater::PeriodicUpdater (int period_ms)
{
	if (PeriodicUpdater::_singleton != 0)
		throw Exception ("PeriodicUpdater is a signleton, and can be instantiated only once");
	PeriodicUpdater::_singleton = this;

	_timer = new QTimer (this);
	QObject::connect (_timer, SIGNAL (timeout()), this, SLOT (timeout()));
	_timer->start (period_ms);
}


PeriodicUpdater::~PeriodicUpdater()
{
	_timer->stop();
	PeriodicUpdater::_singleton = 0;
}


void
PeriodicUpdater::schedule (Receiver* receiver, Thread thread)
{
	_set.insert (std::make_pair (receiver, Shared<Backtrace> (new Backtrace())));
}


void
PeriodicUpdater::forget (Receiver* receiver, Thread thread)
{
	_set_mutex.lock();
	for (Set::iterator i = _set.begin(); i != _set.end(); )
		if (i->first == receiver)
		{
			Set::iterator k = i;
			++k;
			_set.erase (i);
			i = k;
		}
		else
			++i;
//	_set.erase (receiver);
	_set_mutex.unlock();
}


void
PeriodicUpdater::timeout()
{
	if (_set_mutex.try_lock())
	{
		for (Set::iterator i = _set.begin(); i != _set.end(); ++i)
		{
			i->first->periodic_update();
		}
		_set.clear();
		_set_mutex.unlock();
	}
}

