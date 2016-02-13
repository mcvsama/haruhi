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
#include <cmath>

// Local:
#include "crossing_wave.h"


namespace Haruhi {
namespace DSP {

CrossingWave::CrossingWave() noexcept:
	Wave (false)
{ }


CrossingWave::CrossingWave (Wave* from, Wave* to, std::size_t transition_samples) noexcept:
	Wave (false),
	_wave_from (from),
	_wave_to (to),
	_transition_samples (transition_samples)
{ }


CrossingWave&
CrossingWave::operator= (CrossingWave const& other) noexcept
{
	_state = other._state;
	_wave_from = other._wave_from;
	_wave_to = other._wave_to;
	_transition_samples = other._transition_samples;
	_current_sample = other._current_sample;
	return *this;
}


void
CrossingWave::start() noexcept
{
	_state = Running;
	_current_sample = 0;
}


void
CrossingWave::advance (std::size_t samples) noexcept
{
	if (_state == Running)
	{
		_current_sample += samples;

		if (_current_sample >= _transition_samples)
			_state = Finished;
	}
}


void
CrossingWave::reset() noexcept
{
	_state = NotStarted;
	_current_sample = 0;
}


Sample
CrossingWave::operator() (Sample register phase, Sample frequency, std::size_t sample) const noexcept
{
	if (_current_sample < _transition_samples)
	{
		Sample from = (*_wave_from)(phase, frequency, 0);
		Sample to = (*_wave_to)(phase, frequency, 0);

		// Should be expotential in theory, but linear is way faster.
		auto i = std::min (_current_sample + sample, _transition_samples);
		auto ratio = static_cast<Sample> (i) / static_cast<Sample> (_transition_samples);

		return (1.0 - ratio) * from + ratio * to;
	}
	else
		return (*_wave_to)(phase, frequency, 0);
}

} // namespace DSP
} // namespace Haruhi

