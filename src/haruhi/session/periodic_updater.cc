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
#include <set>

// Qt:
#include <QtCore/QTimer>

// Haruhi:
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/exception.h>

// Local:
#include "periodic_updater.h"


namespace Haruhi {

PeriodicUpdater* PeriodicUpdater::_singleton = 0;


void
PeriodicUpdater::Receiver::schedule_for_update()
{
	PeriodicUpdater::singleton()->schedule (this);
}


void
PeriodicUpdater::Receiver::forget_about_update()
{
	PeriodicUpdater::singleton()->forget (this);
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
PeriodicUpdater::schedule (Receiver* receiver)
{
	_set_mutex.lock();
	_set.insert (receiver);
	_set_mutex.unlock();
}


void
PeriodicUpdater::forget (Receiver* receiver)
{
	_set_mutex.lock();
	_set.erase (receiver);
	_set_mutex.unlock();
}


void
PeriodicUpdater::timeout()
{
	Set copy;

	_set_mutex.lock();
	copy = _set;
	_set.clear();
	_set_mutex.unlock();

	for (auto w: copy)
		w->periodic_update();
}

} // namespace Haruhi

