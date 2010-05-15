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

#ifndef HARUHI__CONTROLLER_PROXY_H__INCLUDED
#define HARUHI__CONTROLLER_PROXY_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/periodic_updater.h>
#include <haruhi/core/event.h>
#include <haruhi/core/event_port.h>
#include <haruhi/utility/saveable_state.h>


/**
 * Proxy between event port and int parameter.
 * Also handles UI widget.
 */
class ControllerProxy:
	public SaveableState
{
  public:
	/**
	 * Client class. Knobs and other controls
	 * must derive this class to be able to be paired
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
	 * \param	value: Current value. Becomes also default value for this ControllerProxy.
	 */
	ControllerProxy (Core::EventPort* event_port, int volatile* parameter, int volatile* smoothing_parameter, int limit_min, int limit_max, int value);

	Config*
	config() { return &_config; }

	Core::EventPort*
	event_port() const { return _event_port; }

	int volatile*
	parameter() const { return _parameter; }

	int volatile*
	smoothing_parameter() const { return _smoothing_parameter; }

	int
	value() const { return atomic (*_parameter); }

	void
	set_value (int value) { atomic (*_parameter) = value; }

	int
	smoothing_value() const { return atomic (*_smoothing_parameter); }

	void
	set_smoothing_value (int value)
	{
		if (_smoothing_parameter)
			atomic (*_smoothing_parameter) = value;
	}

	/**
	 * Resets to default value.
	 */
	void
	reset() { set_value (_default_value); }

	/**
	 * Assigns Widget to be notified of parameter updates.
	 */
	void
	set_widget (Widget* widget) { _widget = widget; }

	/**
	 * Tells proxy that config has been updated.
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
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  private:
	Config				_config;
	int					_default_value;
	int volatile*		_parameter;
	int volatile*		_smoothing_parameter;
	Core::EventPort*	_event_port;
	Widget*				_widget;
};

#endif

