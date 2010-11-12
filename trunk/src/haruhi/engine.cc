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
	_wait_semaphore (0),
	_quit (false),
	_panic_pressed (false)
{
	set_sched (Thread::SchedRR, 50);
}


Engine::~Engine()
{
	_quit = true;
	_semaphore.post();
	wait();
}


void
Engine::wait_for_data()
{
	_wait_semaphore.wait();
}


void
Engine::continue_processing()
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
		adjust_master_volume();
		check_panic_button();
		_session->graph()->leave_processing_round();
		_wait_semaphore.post();
		_semaphore.wait();
		if (_quit)
			break;
	}
	while (!_wait_semaphore.try_wait())
		_wait_semaphore.post();
}


void
Engine::adjust_master_volume()
{
	DialControl* master_volume = _session->meter_panel()->master_volume();
	EventBuffer* buffer = _session->audio_backend()->master_volume_port()->event_buffer();
	EventBuffer::EventsMultiset const& events = buffer->events();
	for (EventBuffer::EventsMultiset::const_iterator e = events.begin(); e != events.end(); ++e)
	{
		if ((*e)->event_type() == Event::ControllerEventType)
		{
			ControllerEvent const* controller_event = static_cast<ControllerEvent const*> (e->get());
			// FIXME Use controller_proxy!
			master_volume->setValue (renormalize (controller_event->value(), 0.0, 1.0, Session::MeterPanel::MinVolume, Session::MeterPanel::MaxVolume));
		}
	}
}


void
Engine::check_panic_button()
{
	EventBuffer* buffer = _session->audio_backend()->panic_port()->event_buffer();
	EventBuffer::EventsMultiset const& events = buffer->events();
	for (EventBuffer::EventsMultiset::const_iterator e = events.begin(); e != events.end(); ++e)
	{
		if ((*e)->event_type() == Event::ControllerEventType)
		{
			ControllerEvent const* controller_event = static_cast<ControllerEvent const*> (e->get());
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

