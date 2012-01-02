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

#ifndef HARUHI__GRAPH__AUDIO_BACKEND_H__INCLUDED
#define HARUHI__GRAPH__AUDIO_BACKEND_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>

// Haruhi:
#include <haruhi/config/types.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "backend.h"
#include "audio_port.h"
#include "event_port.h"


namespace Haruhi {

class AudioBackend: public Backend
{
  public:
	// AudioBackend has always fixed ID:
	enum { ID = 0x10000 };

	// Maps audio port to peak level of its audio data:
	typedef std::map<AudioPort*, Sample> LevelsMap;

  public:
	AudioBackend (std::string const& title);

	/**
	 * Tells audio backend that data have been processed and are ready to copy
	 * to audio subsystem. This calls blocks until audio subsystem requests the
	 * data and actual copying is done.
	 */
	virtual void
	data_ready() = 0;

	/**
	 * Returns audio levels for output ports for UV-meters.
	 * Audio levels are recomputed every time in data_ready().
	 * Initially they are set to 0.
	 * \param	levels_map will be cleared and peak output level for each port
	 * 			will be inserted. Level values will be always non-negative.
	 * \entry	Any thread.
	 * 			Caller must lock Graph lock before calling this method and hold
	 * 			it until it finishes operations on resulting levels_map.
	 */
	virtual void
	peak_levels (LevelsMap& levels_map) = 0;

	/**
	 * Gets master volume level.
	 * \threadsafe (uses atomic operations)
	 */
	virtual Sample
	master_volume() const;

	/**
	 * Sets master volume level. All output data will be attenuated using this value.
	 * \threadsafe (uses atomic operations)
	 */
	virtual void
	set_master_volume (Sample volume);

	/**
	 * Returns pointer to master volume port.
	 */
	virtual EventPort*
	master_volume_port() const = 0;

	/**
	 * Returns pointer to panic port.
	 */
	virtual EventPort*
	panic_port() const = 0;

	/**
	 * Creates input port with default name.
	 */
	virtual void
	create_input() = 0;

	/**
	 * Creates input port with given name.
	 */
	virtual void
	create_input (QString const& name) = 0;

	/**
	 * Creates output port with default name.
	 */
	virtual void
	create_output() = 0;

	/**
	 * Creates output port with given name.
	 */
	virtual void
	create_output (QString const& name) = 0;

  private:
	Atomic<Sample> _master_volume;
};

} // namespace Haruhi

#endif

