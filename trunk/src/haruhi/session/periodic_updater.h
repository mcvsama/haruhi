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

#ifndef HARUHI__SESSION__PERIODIC_UPDATER_H__INCLUDED
#define HARUHI__SESSION__PERIODIC_UPDATER_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtCore/QTimer>
#include <QtCore/QObject>

// Haruhi:
#include <haruhi/utility/mutex.h>


namespace Haruhi {

/**
 * PeriodicUpdater calls periodically update() method
 * on all queued widgets. It's useful for UI updates
 * where no instant reaction is needed (knobs and other MIDI controls).
 */
class PeriodicUpdater: public QObject
{
	Q_OBJECT

  public:
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
		 * \threadsafe
		 */
		void
		schedule_for_update();

		/**
		 * Removes this object from PeriodicUpdater queue.
		 * \threadsafe
		 */
		void
		forget_about_update();
	};

  private:
	typedef std::set<Receiver*> Set;

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
	 * \threadsafe
	 */
	void
	schedule (Receiver* receiver);

	/**
	 * Removes widget from set.
	 * \threadsafe
	 */
	void
	forget (Receiver* receiver);

  private slots:
	void
	timeout();

  private:
	static PeriodicUpdater*	_singleton;
	Set						_set;
	Mutex					_set_mutex;
	QTimer*					_timer;
};

} // namespace Haruhi

#endif

