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

#ifndef HARUHI__WIDGETS__CONTROLLER_H__INCLUDED
#define HARUHI__WIDGETS__CONTROLLER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/components/event_backend/event_backend.h>


namespace Haruhi {

class UnitBay;


/**
 * Base class for UI widgets that represent MIDI controllers.
 */
class Controller:
	public ControllerProxy::Widget,
	public EventBackend::Learnable
{
  public:
	Controller (ControllerProxy* controller_proxy);

	virtual ~Controller();

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
	 * Returns associated ControllerProxy.
	 */
	ControllerProxy*
	controller_proxy() const { return _controller_proxy; }

	/**
	 * \entry	Qt thread only.
	 */
	void
	start_learning();

	/**
	 * \entry	Qt thread only.
	 */
	void
	stop_learning();

  protected:
	/**
	 * Called whenever learning is started/stopped.
	 */
	virtual void
	learning_state_changed() { }

	/**
	 * Returns true if Controller is in 'learning' mode.
	 */
	bool
	learning() { return atomic (_learning); }

  private:
	// EventBackend::Learnable API:
	void
	learned_port (EventBackend::EventTypes, Core::EventPort*);

  private:
	ControllerProxy*	_controller_proxy;
	UnitBay*			_unit_bay;
	bool				_learning;
};

} // namespace Haruhi

#endif

