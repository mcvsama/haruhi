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

#ifndef HARUHI__GRAPH__AUDIO_BACKEND_H__INCLUDED
#define HARUHI__GRAPH__AUDIO_BACKEND_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Haruhi:
#include <haruhi/config/types.h>

// Local:
#include "backend.h"
#include "audio_port.h"


namespace Haruhi {

class AudioBackend: public Backend
{
  public:
	// Maps audio port to peak level of its audio data:
	typedef std::map<AudioPort*, Sample> LevelsMap;

  public:
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

  private:
	Sample	_master_volume;
};

} // namespace Haruhi

#endif

