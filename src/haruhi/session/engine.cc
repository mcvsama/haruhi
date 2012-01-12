/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <haruhi/session/session.h>
#include <haruhi/utility/thread.h>
#include <haruhi/utility/semaphore.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "engine.h"


namespace Haruhi {

Engine::Engine (Session* session):
	_session (session),
	_quit (false),
	_panic_pressed (false)
{
	set_sched (Thread::SchedFIFO, 50);
}


Engine::~Engine()
{
	_quit.store (true);
	wait();
}


void
Engine::run()
{
	for (;;)
	{
		_session->graph()->enter_processing_round();
		_session->graph()->audio_backend()->sync();
		adjust_master_volume();
		check_panic_button();
		_session->graph()->leave_processing_round();
		_session->update_level_meters();
		_session->graph()->audio_backend()->data_ready();
		if (_quit.load())
			break;
	}
}


void
Engine::adjust_master_volume()
{
	EventBuffer* buffer = _session->graph()->audio_backend()->master_volume_port()->event_buffer();
	EventBuffer::Events const& events = buffer->events();
	for (auto& e: events)
	{
		if (e->event_type() == Event::ControllerEventType)
		{
			ControllerEvent const* controller_event = static_cast<ControllerEvent const*> (e.get());
			_session->set_master_volume (controller_event->value());
		}
	}
}


void
Engine::check_panic_button()
{
	EventBuffer* buffer = _session->graph()->audio_backend()->panic_port()->event_buffer();
	EventBuffer::Events const& events = buffer->events();
	for (auto e: events)
	{
		if (e->event_type() == Event::ControllerEventType)
		{
			ControllerEvent const* controller_event = static_cast<ControllerEvent const*> (e.get());
			if (controller_event->value() >= 0.5 && _panic_pressed == false)
			{
				_panic_pressed = true;
				_session->graph()->panic();
			}
			else if (controller_event->value() < 0.5)
				_panic_pressed = false;
		}
	}
}

} // namespace Haruhi

