/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
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
#include <stack>

// Haruhi:
#include <haruhi/dsp/wave.h>
#include <haruhi/dsp/wavetable.h>
#include <haruhi/dsp/fft_filler.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/memory.h>

// Local:
#include "wave_computer.h"


namespace MikuruPrivate {

WaveComputer::WaveComputer():
	_wave (0),
	_current (0),
	_current_n (0),
	_start (0),
	_exit (0),
	_exited (0)
{
	// Stack size depends on distro and CFLAGS used for fftw.
	// Let's safely assume that 1MB is fine:
	set_stack_size (1024 * 1024);
	_wavetables[0] = new DSP::Wavetable();
	_wavetables[1] = new DSP::Wavetable();
	start();
}


WaveComputer::~WaveComputer()
{
	stop();
	delete _wavetables[0];
	delete _wavetables[1];
}


void
WaveComputer::update (Shared<DSP::Wave> const& wave)
{
	_wave_mutex.lock();
	_wave = wave;
	_wave_mutex.unlock();

	while (_start.try_wait())
		;
	_start.post();
}


void
WaveComputer::stop()
{
	_exit.post();
	_start.post();
	_exited.wait();
}


void
WaveComputer::run()
{
	while (true)
	{
		// Wait for start signal:
		_start.wait();
		// Check if exit has been requested:
		if (_exit.try_wait())
			break;

		_wave_mutex.lock();
		Shared<DSP::Wave> wave = _wave;
		_wave_mutex.unlock();

		if (&*wave)
		{
			int n = (_current_n + 1) % 2;
			DSP::FFTFiller (&*wave, true).fill (_wavetables[n], 4096);
			_current_n = n;

			atomic (_current) = _wavetables[n];

			// Emit configured signal:
			finished (wave);
		}
	}
	_exited.post();
}

} // namespace MikuruPrivate

