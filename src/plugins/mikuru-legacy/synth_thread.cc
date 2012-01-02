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
#include <set>

// Haruhi:
#include <haruhi/utility/thread.h>

// Local:
#include "mikuru.h"
#include "voice.h"
#include "synth_thread.h"
#include "general.h"


namespace MikuruPrivate {

SynthThread::SynthThread (Mikuru* mikuru):
	_mikuru (mikuru),
	_exit (0),
	_start (0),
	_done (0)
{
	if (_mikuru->graph())
		_mikuru->graph()->lock();
	std::size_t bs = _mikuru->graph()->buffer_size();
	_voice_commons = new VoiceCommons (bs);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
	// 32k stack will be sufficient:
	set_stack_size (32 * 1024);
	set_sched (Thread::SchedFIFO, 50);
}


SynthThread::~SynthThread()
{
	wait();
	if (_mikuru->graph())
		_mikuru->graph()->lock();
	while (!_voices.empty())
		(*_voices.begin())->set_thread (0);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
	delete _voice_commons;
}


void
SynthThread::synthesize()
{
	_start.post();
}


void
SynthThread::join_synthesized() const
{
	_done.wait();
}


void
SynthThread::exit()
{
	_exit.post();
	_start.post();
}


void
SynthThread::resize_buffers (std::size_t buffers_size)
{
	if (_mikuru->graph())
		_mikuru->graph()->lock();
	voice_commons()->resize_buffers (buffers_size);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
SynthThread::run()
{
	while (true)
	{
		_start.wait();

		// Check if exit has been requested:
		if (_exit.try_wait())
			break;

		// Mixin voices:
		for (Voices::iterator v = _voices.begin(); v != _voices.end(); ++v)
			(*v)->process();

		_done.post();
	}
}

} // namespace MikuruPrivate

