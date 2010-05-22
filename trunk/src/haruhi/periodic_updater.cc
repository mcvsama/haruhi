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

	_current_sets[QtThread] = _sets[QtThread] + 0;
	_current_sets[GraphThread] = _sets[GraphThread] + 0;

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
	Set* set = atomic (_current_sets[thread]);
	set->insert (std::make_pair (receiver, Shared<Backtrace> (new Backtrace())));
}


void
PeriodicUpdater::forget (Receiver* receiver, Thread thread)
{
	Set* set = atomic (_current_sets[thread]);
	for (Set::iterator k = set->begin(); k != set->end(); ++k)
		if (k->first == receiver)
		{
			set->erase (k);
			return;
		}
//	set->erase (receiver);
}


void
PeriodicUpdater::timeout()
{
	Set (*s[_ThreadSize]);
	for (int i = 0; i < _ThreadSize; ++i)
		s[i] = atomic (_current_sets[i]);

	for (int i = 0; i < _ThreadSize; ++i)
		atomic (_current_sets[i]) = switch_set (s[i], _sets[i]);

	for (int i = 0; i < _ThreadSize; ++i)
	{
		for (Set::iterator r = s[i]->begin(); r != s[i]->end(); ++r)
		{
			std::clog << *r->second << "\n";
			r->first->periodic_update();
		}
		s[i]->clear();
	}
}


PeriodicUpdater::Set*
PeriodicUpdater::switch_set (Set* set, Set* sets)
{
	if (set == sets + 0)
		return sets + 1;
	return sets + 0;
}

