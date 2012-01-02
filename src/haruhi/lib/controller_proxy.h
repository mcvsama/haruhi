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

#ifndef HARUHI__LIB__CONTROLLER_PROXY_H__INCLUDED
#define HARUHI__LIB__CONTROLLER_PROXY_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/graph/event.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/lib/controller_param.h>
#include <haruhi/session/periodic_updater.h>
#include <haruhi/utility/signal.h>


namespace Haruhi {

/**
 * Proxy between event port, ui widget and controller parameter.
 *
 * When there is event coming from event port, controlled parameter is
 * updated right away, and UI widget is updated from UI thread with PeriodicUpdater
 * (that means it will be updated on next PeriodicUpdater round).
 * When UI widget sends change events, controlled parameter is updated right away.
 */
class ControllerProxy: public Signal::Receiver
{
  public:
	/**
	 * Client class. Knobs and other controls
	 * must inherit this class in order to be able to be paired
	 * with ControllerProxy.
	 *
	 * This means that class inheriting Widget effectively
	 * become PeriodicUpdater's receiver and will periodically
	 * receive periodic_update() events from UI thread.
	 */
	class Widget: public PeriodicUpdater::Receiver
	{
	  public:
		/**
		 * Return true if user is controlling the widget right now
		 * (eg. moving the knob dial).
		 *
		 * Tells ControllerProxy that it should ignore input events.
		 * Default implementation always returns false.
		 */
		virtual bool
		user_override();
	};

  public:
	/**
	 * \param	event_port is event port connected to this proxy.
	 *			Proxy does not take ownership of the port.
	 * \param	param is controller param controlled by this proxy.
	 *			Proxy does not take ownership of the param.
	 */
	ControllerProxy (EventPort* event_port, ControllerParam* param);

	~ControllerProxy();

	EventPort*
	event_port() const;

	ControllerParam*
	param() const;

	/**
	 * Assigns Widget to be notified of parameter updates.
	 * \entry	any thread
	 */
	void
	set_widget (Widget* widget);

	/**
	 * Processes events from assigned EventPort.
	 * Calls process_event() for the most recent controller value in the buffer.
	 */
	void
	process_events();

	/**
	 * Processes given event - propagates changes to controlled
	 * parameter and requests periodic-update on widget.
	 */
	void
	process_event (ControllerEvent const*);

	/**
	 * Set value coming from the widget.
	 * Apply curves and limits.
	 */
	void
	set_value_from_widget (int value);

	/**
	 * Set absolute value.
	 * Do not apply any curves to the parameter.
	 */
	void
	set_absolute_value (int value);

	/**
	 * Reset param to default value.
	 */
	void
	reset_value();

  public:
	/**
	 * Emited when VoiceControllerEvent is encountered
	 * in process_events().
	 */
	Signal::Emiter2<VoiceControllerEvent const*, int> on_voice_controller_event;

  private:
	/**
	 * Called back when param value or adapter config
	 * changes. Calls widget to update itself.
	 */
	void
	param_changed();

  private:
	ControllerParam*	_param;
	EventPort*			_event_port;
	Widget*				_widget;
};


inline bool
ControllerProxy::Widget::user_override()
{
	return false;
}


inline EventPort*
ControllerProxy::event_port() const
{
	return _event_port;
}


inline ControllerParam*
ControllerProxy::param() const
{
	return _param;
}


inline void
ControllerProxy::set_widget (Widget* widget)
{
	_widget = widget;
}


inline void
ControllerProxy::process_event (ControllerEvent const* event)
{
	if (_widget && _widget->user_override())
		return;
	param()->set_from_event (event->value());
}


inline void
ControllerProxy::set_value_from_widget (int value)
{
	param()->set_from_widget (value);
}


inline void
ControllerProxy::set_absolute_value (int value)
{
	param()->set (value);
}


inline void
ControllerProxy::reset_value()
{
	param()->reset();
}


inline void
ControllerProxy::param_changed()
{
	if (_widget)
		_widget->schedule_for_update();
}

} // namespace Haruhi

#endif

