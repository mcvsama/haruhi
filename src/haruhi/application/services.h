/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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
#include <functional>

// Qt:
#include <QObject>
#include <QEvent>
#include <QApplication>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/work_performer.h>


namespace Haruhi {

/**
 * Private API, but can't put it inside of Services class,
 * because MOC won't be able to handle it.
 */
class CallOutDispatcher: public QObject
{
	Q_OBJECT

  private:
	/**
	 * Receive CallOut events.
	 */
	void
	customEvent (QEvent*) override;
};


/**
 * Common public services.
 */
class Services
{
  public:
	/**
	 * Allows calling out given function from within main Qt event queue.
	 * You can use std::bind() as function callback.
	 */
	class CallOutEvent: public QEvent
	{
	  public:
		CallOutEvent (std::function<void()> callback);

		/**
		 * Cancel CallOut. Prevent calling callback from call_out() method.
		 * It's safe to call this method from the callback.
		 */
		void
		cancel();

		/**
		 * Call the callback, unless CallOut has been cancelled.
		 */
		void
		call_out();

	  private:
		bool					_cancelled;
		std::function<void()>	_callback;
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
	hi_priority_work_performer();

	/**
	 * Return normal work performer.
	 */
	static WorkPerformer*
	lo_priority_work_performer();

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
	call_out (std::function<void()> callback);

	/**
	 * Return number of pixels per point on the screen. Takes into account
	 * screen DPI reported by the Qt.
	 */
	static float
	x_pixels_per_point();

	/**
	 * Return number of pixels per point on the screen. Takes into account
	 * screen DPI reported by the Qt.
	 */
	static float
	y_pixels_per_point();

	/**
	 * Return number of pixels per one em, that is one standard line of text
	 * made with default font returned by QApplication.
	 */
	static float
	y_pixels_per_em();

	/**
	 * Master UI scaling factor. Decides what x_pixels_per_point() and y_pixels_per_point()
	 * will actually return.
	 */
	static float
	master_ui_scaling_factor();

  private:
	static Unique<WorkPerformer>		_hi_priority_work_performer;
	static Unique<WorkPerformer>		_lo_priority_work_performer;
	static Unique<CallOutDispatcher>	_call_out_dispatcher;
};


inline
Services::CallOutEvent::CallOutEvent (std::function<void()> callback):
	QEvent (QEvent::User),
	_cancelled (false),
	_callback (callback)
{ }


inline void
Services::CallOutEvent::cancel()
{
	_cancelled = true;
}


inline void
Services::CallOutEvent::call_out()
{
	if (!_cancelled)
		_callback();
}


inline WorkPerformer*
Services::hi_priority_work_performer()
{
	return _hi_priority_work_performer.get();
}


inline WorkPerformer*
Services::lo_priority_work_performer()
{
	return _lo_priority_work_performer.get();
}


namespace ScreenLiterals {

/**
 * Return value in pixels that give 1 point.
 */
inline float
operator"" _screen_pt (long double pt)
{
	return Services::x_pixels_per_point() * pt;
}


/**
 * Return value in pixels that give 1 point.
 */
inline float
operator"" _screen_pt (unsigned long long pt)
{
	return (pt * 1.0_screen_pt);
}


/**
 * Return value in pixels that give 1 mm.
 */
inline float
operator"" _screen_mm (long double mm)
{
	return Services::x_pixels_per_point() * 72 / 25.4 * mm;
}


/**
 * Return value in pixels that give 1 point.
 */
inline float
operator"" _screen_mm (unsigned long long mm)
{
	return (mm * 1.0_screen_mm);
}


/**
 * Return value in pixels that give 1 em (1 line-height).
 */
inline float
operator"" _em (long double em)
{
	return Services::y_pixels_per_em() * em;
}


/**
 * Return value in pixels that give 1 em (1 line-height).
 */
inline float
operator"" _em (unsigned long long em)
{
	return (em * 1.0_em);
}

} // namespace ScreenLiterals

} // namespace Haruhi

#endif

