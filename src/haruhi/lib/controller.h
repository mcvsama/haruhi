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

#ifndef HARUHI__LIB__CONTROLLER_H__INCLUDED
#define HARUHI__LIB__CONTROLLER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/graph/event_backend.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/utility/atomic.h>


namespace Haruhi {

class UnitBay;

/**
 * Base class for UI widgets that represent (MIDI) controllers.
 *
 * Controller should be always unregistered from PeriodicUpdater
 * before ControllerProxies are deleted, to prevent race conditions from PeriodicUpdater.
 * Since Controller has its own ControllerProxy, This is done automatically for you.
 *
 * Classes that inherit Controller must implement periodic_update() method (by PeriodicUpdater)
 * that should read controlled param and update itself according to its value.
 */
class Controller:
	public ControllerProxy::Widget,
	public Signal::Receiver
{
  public:
	Controller (EventPort* event_port, ControllerParam* controller_param);

	virtual ~Controller();

	/**
	 * Returns associated ControllerParam.
	 */
	ControllerParam*
	param() const { return _controller_proxy.param(); }

	/**
	 * Returns associated EventPort.
	 */
	EventPort*
	event_port() const { return _controller_proxy.event_port(); }

	/**
	 * Associate this Controller with UnitBay, so widget can
	 * create a list of ports it can connect to, for example in popup menu.
	 */
	void
	set_unit_bay (UnitBay* unit_bay) { _unit_bay = unit_bay; }

	/**
	 * Returns associated UnitBay.
	 */
	UnitBay*
	unit_bay() const { return _unit_bay; }

	/**
	 * Returns ControllerProxy owned by this Controller.
	 */
	ControllerProxy&
	controller_proxy() { return _controller_proxy; }

	ControllerProxy const&
	controller_proxy() const { return _controller_proxy; }

	/**
	 * Puts controller into (MIDI) learning mode.
	 * Requires that controller has assigned EventPort.
	 * \entry	UI thread only.
	 */
	void
	start_learning();

	/**
	 * Stops learning mode.
	 * Requires that controller has assigned EventPort.
	 * \entry	UI thread only.
	 */
	void
	stop_learning();

	/**
	 * Processes events on ControllerProxy unless user is interacting
	 * with controller's UI at the moment.
	 * You should call this method instead of accessing ControllerProxy.
	 */
	void
	process_events();

	/**
	 * Processes given event on ControllerProxy unless user is interacting
	 * with controller's UI at the moment.
	 * You should call this method instead of accessing ControllerProxy.
	 */
	void
	process_event (ControllerEvent const*);

	/**
	 * Returns true if user is moving dial with the mouse
	 * at the moment. By default returns false.
	 */
	virtual bool
	mouse_pressed() { return false; }

  protected:
	/**
	 * Called whenever learning is started/stopped.
	 * To be reimplemented in subclass.
	 */
	virtual void
	learning_state_changed() { }

	/**
	 * Returns true if Controller is in 'learning' mode.
	 */
	bool
	learning() { return _learning.load(); }

  private:
	/**
	 * Callback from Port's learned_connection_signal.
	 * Updates UI and learning state. It's not defined from within
	 * what thread this method will be called.
	 */
	void
	learned_connection (EventBackend::EventTypes, EventPort*);

  private:
	ControllerProxy	_controller_proxy;
	UnitBay*		_unit_bay;
	Atomic<bool>	_learning;
};

} // namespace Haruhi

#endif

