/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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
#include <haruhi/config/all.h>
#include <haruhi/utility/pool_allocator.h>

// Local:
#include "adsr.h"


namespace Haruhi {

namespace DSP {

POOL_ALLOCATOR_FOR (ADSR)


ADSR::ADSR (unsigned int delay, unsigned int attack, unsigned int attack_hold, unsigned int decay, Sample sustain, unsigned int sustain_hold,
			unsigned int release, bool sustain_enabled, bool forced_release):
	_delay (delay),
	_attack (attack),
	_attack_hold (attack_hold),
	_decay (decay),
	_sustain (sustain),
	_sustain_hold (sustain_hold),
	_release (release),
	_sustain_enabled (sustain_enabled),
	_forced_release (forced_release),
	_released (false),
	_release_at (0.0f),
	_phase (Delay),
	_p (0)
{
}


void
ADSR::release() noexcept
{
	_released = true;
	if (_forced_release)
	{
		_p = 0;
		_phase = Release;
	}
}


void
ADSR::fill (Sample* begin, Sample* end) noexcept
{
	Sample* cur = begin;
	while (cur != end)
	{
		switch (_phase)
		{
			case Delay:
				_release_at = 0.0f;
				while (cur != end && _p < _delay)
				{
					*cur++ = 0.0f;
					_p += 1;
				}
				if (_p >= _delay)
				{
					_p = 0;
					_phase = Attack;
				}
				break;

			case Attack:
				while (cur != end && _p < _attack)
				{
					*cur++ = _release_at = 1.0f * _p / _attack;
					_p += 1;
				}
				if (_p >= _attack)
				{
					_p = 0;
					_phase = AttackHold;
				}
				break;

			case AttackHold:
				while (cur != end && _p < _attack_hold)
				{
					*cur++ = 1.0f;
					_p += 1;
				}
				if (_p >= _attack_hold)
				{
					_p = 0;
					_phase = Decay;
				}
				break;

			case Decay:
				while (cur != end && _p < _decay)
				{
					*cur++ = _release_at = 1.0f - (1.0f - _sustain) * _p / _decay;
					_p += 1;
				}
				if (_p >= _decay)
				{
					_p = 0;
					_phase = Sustain;
				}
				break;

			case Sustain:
				if (!_sustain_enabled || _released)
				{
					_p = 0;
					_phase = SustainHold;
					_release_at = _sustain;
				}
				else
				{
					_release_at = _sustain;
					while (cur != end)
						*cur++ = _sustain;
				}
				break;

			case SustainHold:
				while (cur != end && _p < _sustain_hold)
				{
					*cur++ = _sustain;
					_p += 1;
				}
				if (_p >= _sustain_hold)
				{
					_p = 0;
					_phase = Release;
				}
				break;

			case Release:
				while (cur != end && _p < _release)
				{
					*cur++ = _release_at - _release_at * _p / _release;
					_p += 1;
				}
				if (_p >= _release)
				{
					_p = 0;
					_phase = Finished;
				}
				break;

			case Finished:
				_release_at = 0.0f;
				while (cur != end)
					*cur++ = 0.0f;
				break;
		}
	}
}

} // namespace DSP

} // namespace Haruhi

