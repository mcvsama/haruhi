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

#ifndef HARUHI__APPLICATION__SERVICES_H__INCLUDED
#define HARUHI__APPLICATION__SERVICES_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <boost/function.hpp>

// Qt:
#include <QtCore/QObject>
#include <QtCore/QEvent>
#include <QtGui/QApplication>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/work_performer.h>


namespace Haruhi {

namespace P {

class CallOutDispatcher: public QObject
{
	Q_OBJECT

  private:
	/**
	 * Receive CallOut events.
	 */
	void
	customEvent (QEvent*);
};

} // namespace P


class Services
{
  public:
	/**
	 * Allows calling out given function from within main Qt event queue.
	 * You can use boost::bind() as function callback.
	 */
	class CallOutEvent: public QEvent
	{
	  public:
		CallOutEvent (boost::function<void()> callback):
			QEvent (QEvent::User),
			_cancelled (false),
			_callback (callback)
		{ }

		/**
		 * Cancel CallOut. Prevent calling callback from call_out() method.
		 * It's safe to call this method from the callback.
		 */
		void
		cancel() { _cancelled = true; }

		/**
		 * Call the callback, unless CallOut has been cancelled.
		 */
		void
		call_out()
		{
			if (!_cancelled)
				_callback();
		}

	  private:
		bool					_cancelled;
		boost::function<void()> _callback;
	};

  public:
	/**
	 * Initialize services.
	 * Call AFTER initialization of QApplication.
	 */
	static void
	initialize();

	/**
	 * Deinit.
	 * Frees resources. Call it BEFORE deletion of QApplication.
	 */
	static void
	deinitialize();

	/**
	 * Return RT-prioritized work performer.
	 */
	static WorkPerformer*
	hi_priority_work_performer() { return _hi_priority_work_performer; }

	/**
	 * Return normal work performer.
	 */
	static WorkPerformer*
	lo_priority_work_performer() { return _lo_priority_work_performer; }

	/**
	 * Return number of detected processor cores.
	 */
	static unsigned int
	detected_cores();

	/**
	 * Return vector of compiled-in feature names.
	 */
	static std::vector<const char*>
	features();

	/**
	 * Register given callback to be called from within main Qt event queue.
	 * \return	CallOutEvent object. This object is deleted after call has been made.
	 */
	static CallOutEvent*
	call_out (boost::function<void()> callback);

  private:
	static WorkPerformer*			_hi_priority_work_performer;
	static WorkPerformer*			_lo_priority_work_performer;
	static signed int				_detected_cores;
	static P::CallOutDispatcher*	_call_out_dispatcher;
};

} // namespace Haruhi

#endif

