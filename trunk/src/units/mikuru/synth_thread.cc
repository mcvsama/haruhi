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
#include <set>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/dsp/smoother.h>
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
	_buffer_1 = new Haruhi::AudioBuffer (bs);
	_buffer_2 = new Haruhi::AudioBuffer (bs);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
	// 512k stack will be sufficient:
	set_stack_size (512 * 1024);
	set_sched (Thread::SchedRR, 50);
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
	delete _buffer_1;
	delete _buffer_2;
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
	_buffer_1->resize (buffers_size);
	_buffer_2->resize (buffers_size);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
SynthThread::run()
{
	int smoothing_samples = 0;

	while (true)
	{
		_start.wait();

		// Check if exit has been requested:
		if (_exit.try_wait())
			break;

		_buffer_1->clear();
		_buffer_2->clear();

		// Mixin voices:
		for (Voices::iterator v = _voices.begin(); v != _voices.end(); ++v)
			(*v)->mixin (_buffer_1, _buffer_2);

		// Stereo width:
		// TODO smoothing
		float w = std::pow (1.0f - _mikuru->general()->params()->stereo_width.to_f(), M_E);
		Sample o1, o2;
		for (Sample *s1 = _buffer_1->begin(), *s2 = _buffer_2->begin(); s1 != _buffer_1->end(); ++s1, ++s2)
		{
			o1 = *s1;
			o2 = *s2;
			*s1 += w * o2;
			*s2 += w * o1;
		}

		// Panorama:
		int k = 0.001f * _mikuru->general()->params()->panorama.smoothing() * _mikuru->graph()->sample_rate();
		if (smoothing_samples != k)
		{
			smoothing_samples = k;
			_panorama_smoother_1.set_smoothing_samples (smoothing_samples);
			_panorama_smoother_2.set_smoothing_samples (smoothing_samples);
		}

		float f = 0.0;
		f = 1.0f - 1.0f * _mikuru->general()->params()->panorama.get() / Params::General::PanoramaMax;
		f = f > 1.0f ? 1.0 : f;
		_panorama_smoother_1.set_value (f);
		_panorama_smoother_1.multiply (_buffer_1->begin(), _buffer_1->end());

		f = 1.0f - 1.0f * _mikuru->general()->params()->panorama.get() / Params::General::PanoramaMin;
		f = f > 1.0f ? 1.0 : f;
		_panorama_smoother_2.set_value (f);
		_panorama_smoother_2.multiply (_buffer_2->begin(), _buffer_2->end());

		_done.post();
	}
}

} // namespace MikuruPrivate

