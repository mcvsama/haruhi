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

#ifndef HARUHI__LIB__CONTROLLER_PROXY_H__INCLUDED
#define HARUHI__LIB__CONTROLLER_PROXY_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/periodic_updater.h>
#include <haruhi/core/event.h>
#include <haruhi/core/event_port.h>
#include <haruhi/lib/controller_param.h>
#include <haruhi/utility/saveable_state.h>


namespace Haruhi {

/**
 * Proxy between event port and int parameter.
 * Also handles UI widget.
 */
class ControllerProxy: public SaveableState
{
  public:
	/**
	 * Client class. Knobs and other controls
	 * must derive this class in order to be able to be paired
	 * with ControllerProxy.
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

		int
		forward (int in) const;

		int
		reverse (int in) const;

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
	 * 			Proxy does not take ownership of the port.
	 * \param	param is controller param controlled by this proxy.
	 * 			Proxy does not take ownership of the param.
	 */
	ControllerProxy (Core::EventPort* event_port, ControllerParam* param);

	Core::EventPort*
	event_port() const { return _event_port; }

	ControllerParam*
	param() const { return _param; }

	Config*
	config() { return &_config; }

	/**
	 * Assigns Widget to be notified of parameter updates.
	 */
	void
	set_widget (Widget* widget) { _widget = widget; }

	/**
	 * Tells proxy that config has been updated
	 * and widget needs to be updated too.
	 * \entry	Qt thread only.
	 */
	void
	apply_config();

	/**
	 * Processes events from assigned EventPort.
	 * Calls process_event() for the last controller value in the buffer.
	 */
	void
	process_events();

	/**
	 * Processes given event.
	 */
	void
	process_event (Core::ControllerEvent const*);

	/*
	 * SaveableState API
	 * Saves ControllerProxy configuration (curves, etc.)
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  private:
	Config				_config;
	ControllerParam*	_param;
	Core::EventPort*	_event_port;
	Widget*				_widget;
};

} // namespace Haruhi

#endif

