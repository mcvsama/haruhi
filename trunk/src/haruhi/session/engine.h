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

#ifndef HARUHI__ENGINE_H__INCLUDED
#define HARUHI__ENGINE_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/utility/thread.h>
#include <haruhi/utility/semaphore.h>
#include <haruhi/session/session.h>


namespace Haruhi {

class Engine: public Thread
{
  public:
	Engine (Session* session);

	~Engine();

	/**
	 * Waits for processing round completion,
	 * which is done at end of each round inside
	 * run() loop.
	 */
	void
	wait_for_data();

	/**
	 * Tells engine that it should perform processing round
	 * on graph, that is it should execute one run() loop
	 * round.
	 * Returns immediately.
	 *
	 * \threadsafe
	 */
	void
	continue_processing();

  protected:
	/**
	 * Main engine loop. Loops until _quit is set.
	 * Syncs audio backend on each round and
	 * signals wait_for_data() semaphore.
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
	Session*	_session;
	Semaphore	_semaphore;
	Semaphore	_wait_semaphore;
	bool		_quit;
	bool		_panic_pressed;
};

} // namespace Haruhi

#endif

