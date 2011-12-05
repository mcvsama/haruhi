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

#ifndef HARUHI__PLUGINS__YUKI__VOICE_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__VOICE_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/event.h>

// Local:
#include "params.h"
#include "voice_oscillator.h"


namespace Yuki {

class Voice;

typedef std::set<Voice*> Voices;

class Voice
{
  public:
	enum State { NotStarted, Voicing, Dropped, Finished };

  public:
	Voice (Haruhi::VoiceID id, Haruhi::Timestamp timestamp);

	/**
	 * Return voice's ID which came in Haruhi::VoiceEvent.
	 */
	Haruhi::VoiceID
	id() const { return _id; }

	/**
	 * Return voice's timestamp.
	 */
	Haruhi::Timestamp
	timestamp() const { return _timestamp; }

	/**
	 * Return current voice state.
	 */
	State
	state() const { return _state; }

	/**
	 * Drop voice. Voice does not immediately stop sounding.
	 * Use finished() to check if voice generation is really finished.
	 */
	void
	drop();

	/**
	 * Synthesize voice and fill output buffers.
	 * \return	true if buffers were modified, false otherwise.
	 */
	bool
	render (Haruhi::AudioBuffer*, Haruhi::AudioBuffer*);

	/**
	 * Update buffers sizes.
	 */
	void
	graph_updated (unsigned int sample_rate, std::size_t buffer_size);

  public:
	/**
	 * Return older from the two voices by comparing
	 * their timestamps.
	 */
	static Voice*
	return_older (Voice* a, Voice* b)
	{
		if (a->timestamp() < b->timestamp())
			return a;
		return b;
	}

  private:
	Haruhi::VoiceID		_id;
	Haruhi::Timestamp	_timestamp;
	State				_state;
	Params::Voice		_params;
	VoiceOscillator		_vosc;
};

} // namespace Yuki

#endif

