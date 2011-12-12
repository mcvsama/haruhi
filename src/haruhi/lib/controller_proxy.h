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

#ifndef HARUHI__LIB__CONTROLLER_PROXY_H__INCLUDED
#define HARUHI__LIB__CONTROLLER_PROXY_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/graph/event.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/lib/controller_param.h>
#include <haruhi/session/periodic_updater.h>
#include <haruhi/utility/saveable_state.h>
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
class ControllerProxy: public SaveableState
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
	};

	/**
	 * Proxy configuration.
	 * Applies curve and limits to input data.
	 */
	class Config
	{
	  public:
		Config (int limit_min, int limit_max);

		/**
		 * Applies forward transform. Takes input value,
		 * returns curved and limited value.
		 */
		int
		forward (int in) const;

		/**
		 * Applies reverse transform (inverse of what forward() does).
		 */
		int
		reverse (int in) const;

		/**
		 * The same as forward(), but takes input value normalized
		 * into range [0.0, 1.0].
		 */
		int
		forward_normalized (float in) const;

	  private:
		int
		encurve (int in) const;

		int
		decurve (int in) const;

	  public:
		float	curve;
		int		hard_limit_min;
		int		hard_limit_max;
		int		user_limit_min;
		int		user_limit_max;
	};

  public:
	/**
	 * \param	event_port is event port connected to this proxy.
	 *			Proxy does not take ownership of the port.
	 * \param	param is controller param controlled by this proxy.
	 *			Proxy does not take ownership of the param.
	 */
	ControllerProxy (EventPort* event_port, ControllerParam* param);

	EventPort*
	event_port() const;

	ControllerParam*
	param() const;

	Config&
	config();

	Config const&
	config() const;

	/**
	 * Assigns Widget to be notified of parameter updates.
	 * \entry	any thread
	 */
	void
	set_widget (Widget* widget);

	/**
	 * Tells proxy that config has been updated
	 * and widget needs to be updated too.
	 * \entry	Qt thread only.
	 */
	void
	apply_config();

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

	/*
	 * SaveableState API
	 * Saves ControllerProxy configuration (curves, etc.)
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  public:
	/**
	 * Emited when VoiceControllerEvent is encountered
	 * in process_events().
	 */
	Signal::Emiter1<VoiceControllerEvent const*> on_voice_controller_event;

  private:
	Config				_config;
	ControllerParam*	_param;
	EventPort*			_event_port;
	Widget*				_widget;
};


inline int
ControllerProxy::Config::forward (int in) const
{
	return renormalize (encurve (in), hard_limit_min, hard_limit_max, user_limit_min, user_limit_max);
}


inline int
ControllerProxy::Config::reverse (int in) const
{
	return decurve (renormalize (in, user_limit_min, user_limit_max, hard_limit_min, hard_limit_max));
}


inline int
ControllerProxy::Config::forward_normalized (float in) const
{
	return forward (renormalize (in, 0.0f, 1.0f, 1.0f * hard_limit_min, 1.0f * hard_limit_max));
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


inline ControllerProxy::Config&
ControllerProxy::config()
{
	return _config;
}


inline ControllerProxy::Config const&
ControllerProxy::config() const
{
	return _config;
}


inline void
ControllerProxy::set_widget (Widget* widget)
{
	_widget = widget;
}


inline void
ControllerProxy::process_event (ControllerEvent const* event)
{
	// Update parameter:
	param()->set (_config.forward_normalized (event->value()));
	// Schedule update for paired Widget:
	if (_widget)
		_widget->schedule_for_update();
}

} // namespace Haruhi

#endif

