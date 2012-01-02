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

#ifndef HARUHI__PLUGINS__MIKURU__SYNTH_THREAD_H__INCLUDED
#define HARUHI__PLUGINS__MIKURU__SYNTH_THREAD_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Haruhi:
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/one_pole_smoother.h>
#include <haruhi/utility/thread.h>
#include <haruhi/utility/semaphore.h>

// Local:
#include "voice.h"
#include "voice_commons.h"


class Mikuru;


namespace MikuruPrivate {

class SynthThread;

typedef std::set<SynthThread*> SynthThreads;

/**
 * Thread for synthesizing Voices (used for synth. on multiple CPUs/cores).
 * Contains set of Voices. Voice may belong only to one SynthThread.
 */
class SynthThread: public Thread
{
  public:
	/**
	 * Ctor
	 * \entry	Any thread.
	 */
	SynthThread (Mikuru*);

	/**
	 * Dtor
	 * \entry	Any thread.
	 */
	~SynthThread();

	/**
	 * Starts synthesizing Voices. Returns immediately.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	void
	synthesize();

	/**
	 * Waits for thread to synthesize all Voices.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	void
	join_synthesized() const;

	/**
	 * Tells thread to gracefully exit.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	void
	exit();

	/**
	 * You may manipulate/read set of Voices only when thread is not synthesizing.
	 * \returns	set of Voices.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	Voices&
	voices() { return _voices; }

	Voices const&
	voices() const { return _voices; }

	/**
	 * \returns	VoiceCommons for this thread.
	 * \entry	Any thread
	 * \threadsafe
	 */
	VoiceCommons*
	voice_commons() const { return _voice_commons; }

	/**
	 * Resizes audio buffers.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	void
	resize_buffers (std::size_t buffers_size);

  protected:
	void
	run();

  private:
	Mikuru*					_mikuru;
	Voices					_voices;
	VoiceCommons*			_voice_commons;
	Mutex					_processing_voices;

	Semaphore				_exit;
	Semaphore				_start;
	Semaphore				_done;
};

} // namespace MikuruPrivate

#endif

