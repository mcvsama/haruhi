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

#ifndef HARUHI__DSP__ADSR_H__INCLUDED
#define HARUHI__DSP__ADSR_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/pool_allocator.h>


namespace Haruhi {

namespace DSP {

class ADSR
{
	USES_POOL_ALLOCATOR (ADSR)

	enum Phase {
		Delay,
		Attack,
		AttackHold,
		Decay,
		Sustain,
		SustainHold,
		Release,
		Finished
	};

  public:
	/**
	 * \param	sustain_enabled: Sustaining enabled or going directly to release phase?
	 * \param	forced_release: Go directly to release phase on release().
	 */
	ADSR (unsigned int delay, unsigned int attack, unsigned int attack_hold, unsigned int decay, Sample sustain, unsigned int sustain_hold,
		  unsigned int release, bool sustain_enabled, bool forced_release);

	void
	release();

	bool
	released() const;

	bool
	finished() const;

	void
	fill (Sample* begin, Sample* end);

  private:
	// D,A,D,R are sample numbers:
	unsigned int	_delay;
	unsigned int	_attack;
	unsigned int	_attack_hold;
	unsigned int	_decay;
	Sample			_sustain;
	unsigned int	_sustain_hold;
	unsigned int	_release;
	bool			_sustain_enabled;
	bool			_forced_release;
	bool			_released;
	Sample			_release_at;
	// Current ADSR phase:
	Phase			_phase;
	// Phase sample number:
	unsigned int	_p;
};


inline bool
ADSR::released() const
{
	return _phase == Release || _phase == Finished;
}


inline bool
ADSR::finished() const
{
	return _phase == Finished;
}

} // namespace DSP

} // namespace Haruhi

#endif

