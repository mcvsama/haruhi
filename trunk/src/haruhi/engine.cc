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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/utility/thread.h>
#include <haruhi/utility/semaphore.h>
#include <haruhi/session.h>

// Local:
#include "engine.h"


namespace Haruhi {

Engine::Engine (Session* session):
	_session (session),
	_semaphore (0),
	_quit (false)
{
	set_sched (Thread::FIFO, 50);
}


Engine::~Engine()
{
	_quit = true;
	_semaphore.post();
	wait();
}


void
Engine::signal()
{
	_semaphore.post();
}


void
Engine::run()
{
	for (;;)
	{
		_session->graph()->enter_processing_round();
		_session->audio_backend()->sync();
		_session->graph()->leave_processing_round();
		_semaphore.wait();
		if (_quit)
			break;
	}
}

} // namespace Haruhi

