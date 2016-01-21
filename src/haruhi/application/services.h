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

// Lib:
#include <boost/function.hpp>

// Qt:
#include <QtCore/QObject>
#include <QtCore/QEvent>
#include <QtGui/QApplication>
#include <QtGui/QX11Info>

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
	 * You can use boost::bind() as function callback.
	 */
	class CallOutEvent: public QEvent
	{
	  public:
		CallOutEvent (boost::function<void()> callback);

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
	hi_priority_work_performer();

	/**
	 * Return normal work performer.
	 */
	static WorkPerformer*
	lo_priority_work_performer();

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
	 * Master UI scaling factor. Decides what x_pixels_per_point() and y_pixels_per_point()
	 * will actually return.
	 */
	static float
	master_ui_scaling_factor();

  private:
	static WorkPerformer*		_hi_priority_work_performer;
	static WorkPerformer*		_lo_priority_work_performer;
	static signed int			_detected_cores;
	static CallOutDispatcher*	_call_out_dispatcher;
};


inline
Services::CallOutEvent::CallOutEvent (boost::function<void()> callback):
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
	return _hi_priority_work_performer;
}


inline WorkPerformer*
Services::lo_priority_work_performer()
{
	return _lo_priority_work_performer;
}


inline float
Services::x_pixels_per_point()
{
	// 1 point is 1/72 of an inch:
	return QX11Info::appDpiX() / 72.0f * master_ui_scaling_factor();
}


inline float
Services::y_pixels_per_point()
{
	// 1 point is 1/72 of an inch:
	return QX11Info::appDpiY() / 72.0f * master_ui_scaling_factor();
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
	return Services::x_pixels_per_point() * pt;
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
	return Services::x_pixels_per_point() * 72 / 25.4 * mm;
}

} // namespace ScreenLiterals

} // namespace Haruhi

#endif

