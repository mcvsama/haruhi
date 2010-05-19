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
#include <haruhi/session.h>


namespace Haruhi {

class Engine: public Thread
{
  public:
	Engine (Session* session);

	~Engine();

	/**
	 * Waits for processing round completion.
	 */
	void
	wait_for_data();

	/**
	 * Tells engine that it should perform processing round
	 * on graph.
	 * \threadsafe
	 */
	void
	signal();

  protected:
	void
	run();

  private:
	void
	adjust_master_volume();

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

