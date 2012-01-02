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

#ifndef HARUHI__PLUGINS__MIKURU__WAVE_COMPUTER_H__INCLUDED
#define HARUHI__PLUGINS__MIKURU__WAVE_COMPUTER_H__INCLUDED

// Standard:
#include <cstddef>
#include <stack>

// Haruhi:
#include <haruhi/dsp/wave.h>
#include <haruhi/dsp/wavetable.h>
#include <haruhi/utility/thread.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/signal.h>
#include <haruhi/utility/semaphore.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/memory.h>


namespace MikuruPrivate {

namespace DSP = Haruhi::DSP;

/**
 * Contains double-buffered wavetable.
 * Recomputes/fills them in separate thread when
 * ordered with given wave function.
 */
class WaveComputer: public Thread
{
  public:
	WaveComputer();

	~WaveComputer();

	/**
	 * Returns wavetable to use or 0 if not yet available.
	 * \entry	Any thread.
	 */
	DSP::Wavetable*
	wavetable() const { return _current.load(); }

	/**
	 * Recomputes wavetables for new wave.
	 * Wavetable returned by wavetable() function does not change
	 * until switch_wavetable() is called.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	void
	update (Shared<DSP::Wave> const& wave);

	/**
	 * Switches wavetables, if newly computed wavetable is available.
	 * Otherwise does nothing.
	 */
	void
	switch_wavetables();

	/**
	 * Stops computing thread. Must be called before joining.
	 * \entry	Any thread.
	 */
	void
	stop();

  public:
	// Emited when calculations are finished:
	Signal::Emiter1<Shared<DSP::Wave> const&> finished;

  protected:
	void
	run();

  private:
	Shared<DSP::Wave>		_wave;
	Mutex					_wave_mutex;
	DSP::Wavetable*			_wavetables[2];
	Atomic<DSP::Wavetable*>	_current;
	Atomic<int>				_current_n;
	Atomic<int>				_computed_n;
	Semaphore				_start;
	Semaphore				_exit;
	Semaphore				_exited;
};

} // namespace MikuruPrivate

#endif

