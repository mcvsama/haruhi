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

#ifndef HARUHI__SESSION__ENGINE_H__INCLUDED
#define HARUHI__SESSION__ENGINE_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/utility/thread.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/session/session.h>


namespace Haruhi {

class Engine: public Thread
{
  public:
	Engine (Session* session);

	/**
	 * Normal way to stop Engine is to delete it.
	 * Destructor will wait for all operations to stop.
	 */
	~Engine();

  protected:
	/**
	 * Main engine loop. Loops until destructor is called.
	 * Syncs audio backend on each round and tells audio backend
	 * that data is ready for consumption.
	 */
	void
	run();

  private:
	/**
	 * Reads events from audio_backend()->master_volume_port()
	 * and adjusts volume knob.
	 */
	void
	adjust_master_volume();

	/**
	 * Reads events from audio_backend()->panic_port()
	 * and perhaps drops all sounds.
	 */
	void
	check_panic_button();

  private:
	Session*		_session;
	Atomic<bool>	_quit;
	bool			_panic_pressed;
};

} // namespace Haruhi

#endif

