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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__DUMMY_THREAD_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__DUMMY_THREAD_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/utility/thread.h>
#include <haruhi/utility/semaphore.h>


namespace Haruhi {

namespace AudioBackend {

class Backend;

/**
 * Dummy thread for executing processing rounds on Graphs
 * while Backend is not connected to JACK.
 * Implemented as different thread because executing
 * processing rounds on Graph cannot be done from within
 * Qt thread (called by QTimer).
 */
class DummyThread: public Thread
{
  public:
	DummyThread (Backend*);

	/**
	 * Executes one dummy processing round on Backend.
	 */
	void
	execute();

	/**
	 * Quits dummy thread (does not wait for finish, use wait()).
	 */
	void
	quit();

  protected:
	void
	run();

  private:
	Backend*	_backend;
	Semaphore	_semaphore;
	bool		_quit;
};

} // namespace AudioBackend

} // namespace Haruhi

#endif

