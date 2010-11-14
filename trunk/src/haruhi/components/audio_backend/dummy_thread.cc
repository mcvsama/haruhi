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

// Local:
#include "dummy_thread.h"
#include "backend.h"


namespace Haruhi {

namespace AudioBackend {

DummyThread::DummyThread (Backend* backend):
	_backend (backend),
	_semaphore (0),
	_quit (false)
{ }


void
DummyThread::execute()
{
	_semaphore.post();
}


void
DummyThread::quit()
{
	_quit = true;
	_semaphore.post();
}


void
DummyThread::run()
{
	for (;;)
	{
		_semaphore.wait();
		if (_quit)
			break;
		_backend->graph()->enter_processing_round();
		_backend->graph()->leave_processing_round();
	}
}

} // namespace AudioBackend

} // namespace Haruhi

