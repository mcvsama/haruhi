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

#ifndef HARUHI__PERIODIC_UPDATER_H__INCLUDED
#define HARUHI__PERIODIC_UPDATER_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtCore/QTimer>
#include <QtCore/QObject>

// Haruhi:
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/backtrace.h>//XXX
#include <haruhi/utility/memory.h>//XXX


/**
 * PeriodicUpdater calls periodically update() method
 * on all queued widgets. It's useful for UI updates
 * where no instant reaction is needed (knobs and other MIDI controls).
 *
 * For performance reasons it doesn't use mutexes for locking but instead has
 * two queues to be used from within Qt thread and JACK thread
 * (schedule_for_update_from_qt_thread() and schedule_for_update_from_graph_thread()).
 */
class PeriodicUpdater: public QObject
{
	Q_OBJECT

  public:
	enum Thread
	{
		QtThread	= 0,
		GraphThread	= 1,
		_ThreadSize	= 2, // Do not use, required internally
	};

	class Receiver
	{
	  public:
		/**
		 * Receiver MUST NOT call schedule_for_update() or forget_about_update()
		 * or PeriodicUpdater schedule()/forget() inside periodic_update()
		 * as it would lead to deadlock.
		 */
		virtual void
		periodic_update() = 0;

		/**
		 * Schedules this object to be updated by PeriodicUpdater.
		 * \entry	Qt thread only if queue == QtThread, graph thread only if queue == GraphThread.
		 */
		void
		schedule_for_update (Thread thread);

		/**
		 * Removes this object from PeriodicUpdater queue.
		 * \entry	Qt thread only if queue == QtThread, graph thread only if queue == GraphThread.
		 */
		void
		forget_about_update (Thread thread);
	};

  private:
	typedef std::set<std::pair<Receiver*, Shared<Backtrace> > > Set;

  public:
	/**
	 * \param	period_ms is length of period in milliseconds.
	 */
	PeriodicUpdater (int period_ms);

	~PeriodicUpdater();

	static PeriodicUpdater*
	singleton() { return _singleton; }

	/**
	 * Adds widget to set. After update object is removed.
	 * \entry	Qt thread only if queue == QtThread, graph thread only if queue == GraphThread.
	 */
	void
	schedule (Receiver* receiver, Thread thread);

	/**
	 * Removes widget from set.
	 * \entry	Qt thread only if queue == QtThread, graph thread only if queue == GraphThread.
	 */
	void
	forget (Receiver* receiver, Thread thread);

  private slots:
	void
	timeout();

  private:
	Set*
	switch_set (Set* set, Set* sets);

  private:
	static PeriodicUpdater*	_singleton;
	Set						_sets[_ThreadSize][2];
	Set*					_current_sets[_ThreadSize];
	QTimer*					_timer;
};

#endif

