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
#include <haruhi/utility/signal.h>


namespace Haruhi {

class UnitBay;

/**
 * Base class for UI widgets that represent (MIDI) controllers.
 *
 * Controller should be always unregistered from PeriodicUpdater
 * before ControllerProxies are deleted, to prevent race conditions from PeriodicUpdater.
 * If you create a Controller with its own ControllerProxy, this will be done automatically for you.
 *
 * Classes that inherit Controller must implement periodic_update() method (by PeriodicUpdater)
 * that will read controlled param and update itself according to its value.
 */
class Controller:
	public ControllerProxy::Widget,
	public Signal::Receiver
{
  public:
	/**
	 * Create a Controller with its own ControllerProxy inside.
	 */
	Controller (EventPort* event_port, ControllerParam* controller_param);

	/**
	 * Create a Controller with external ControllerProxy.
	 * There can be only one Controller per ControllerProxy.
	 * Remember that the Controller MUST be destroyed first, before ControllerProxy.
	 */
	Controller (ControllerProxy* controller_proxy);

	virtual ~Controller();

	/**
	 * Returns associated ControllerParam.
	 */
	ControllerParam*
	param() const;

	/**
	 * Returns associated EventPort.
	 */
	EventPort*
	event_port() const;

	/**
	 * Associate this Controller with UnitBay, so widget can
	 * create a list of ports it can connect to, for example in popup menu.
	 */
	void
	set_unit_bay (UnitBay* unit_bay);

	/**
	 * Returns associated UnitBay.
	 */
	UnitBay*
	unit_bay() const;

	/**
	 * Return ControllerProxy owned by this Controller.
	 */
	ControllerProxy*
	controller_proxy();

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
	learning();

  private:
	/**
	 * Common construction code.
	 */
	void
	initialize();

	/**
	 * Callback from Port's learned_connection_signal.
	 * Updates UI and learning state. It's not defined from within
	 * what thread this method will be called.
	 */
	void
	learned_connection (EventBackend::EventTypes, EventPort*);

  private:
	ControllerProxy*	_controller_proxy;
	bool				_own_controller_proxy;
	UnitBay*			_unit_bay;
	Atomic<bool>		_learning;

  public:
	/**
	 * Emited when VoiceControllerEvent is encountered
	 * in ControllerProxy::process_events().
	 *
	 * Actually, this is just reference to
	 * ControllerProxy::on_voice_controller_event.
	 */
	Signal::Emiter2<VoiceControllerEvent const*, int>& on_voice_controller_event;
};


inline ControllerParam*
Controller::param() const
{
	return _controller_proxy->param();
}


inline EventPort*
Controller::event_port() const
{
	return _controller_proxy->event_port();
}


inline void
Controller::set_unit_bay (UnitBay* unit_bay)
{
	_unit_bay = unit_bay;
}


inline UnitBay*
Controller::unit_bay() const
{
	return _unit_bay;
}


inline ControllerProxy*
Controller::controller_proxy()
{
	return _controller_proxy;
}


inline bool
Controller::learning()
{
	return _learning.load();
}

} // namespace Haruhi

#endif

