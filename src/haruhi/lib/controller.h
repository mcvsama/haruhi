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

#ifndef HARUHI__LIB__CONTROLLER_H__INCLUDED
#define HARUHI__LIB__CONTROLLER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
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
	public v06::ControllerProxy::Widget,
	public Signal::Receiver
{
  public:
	/**
	 * Create a Controller with its own ControllerProxy inside.
	 */
	Controller (EventPort* event_port, v06::ControllerParam* controller_param);

	/**
	 * Create a Controller with external ControllerProxy.
	 * There can be only one Controller per ControllerProxy.
	 * Remember that the Controller MUST be destroyed first, before ControllerProxy.
	 */
	Controller (v06::ControllerProxy* controller_proxy);

	virtual ~Controller();

	/**
	 * Returns associated ControllerParam.
	 */
	v06::ControllerParam*
	param() const noexcept;

	/**
	 * Returns associated EventPort.
	 */
	EventPort*
	event_port() const noexcept;

	/**
	 * Associate this Controller with UnitBay, so widget can
	 * create a list of ports it can connect to, for example in popup menu.
	 */
	void
	set_unit_bay (UnitBay* unit_bay) noexcept;

	/**
	 * Returns associated UnitBay.
	 */
	UnitBay*
	unit_bay() const noexcept;

	/**
	 * Return ControllerProxy owned by this Controller.
	 */
	v06::ControllerProxy*
	controller_proxy() noexcept;

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
	learning() noexcept;

  private:
	/**
	 * Callback from Port's learned_connection_signal.
	 * Updates UI and learning state. It's not defined from within
	 * what thread this method will be called.
	 */
	void
	learned_connection (EventBackend::EventTypes, EventPort*);

  private:
	v06::ControllerProxy*	_controller_proxy		= nullptr;
	bool					_own_controller_proxy	= false;
	UnitBay*				_unit_bay				= nullptr;
	Atomic<bool>			_learning;

  public:
	/**
	 * Emitted when VoiceControllerEvent is encountered
	 * in ControllerProxy::process_events().
	 *
	 * Actually, this is just reference to
	 * ControllerProxy::on_voice_controller_event.
	 */
	Signal::Emiter<VoiceControllerEvent const*, int>& on_voice_controller_event;
};


inline v06::ControllerParam*
Controller::param() const noexcept
{
	return _controller_proxy->param();
}


inline EventPort*
Controller::event_port() const noexcept
{
	return _controller_proxy->event_port();
}


inline void
Controller::set_unit_bay (UnitBay* unit_bay) noexcept
{
	_unit_bay = unit_bay;
}


inline UnitBay*
Controller::unit_bay() const noexcept
{
	return _unit_bay;
}


inline v06::ControllerProxy*
Controller::controller_proxy() noexcept
{
	return _controller_proxy;
}


inline bool
Controller::learning() noexcept
{
	return _learning.load();
}

} // namespace Haruhi

#endif

