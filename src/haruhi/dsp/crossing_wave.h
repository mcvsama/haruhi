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

#ifndef HARUHI__DSP__CROSSING_WAVE_H__INCLUDED
#define HARUHI__DSP__CROSSING_WAVE_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>


namespace Haruhi {
namespace DSP {

/**
 * Implements a Wave that mixes two other waves, that is makes smooth
 * transition from first wave to second.
 */
class CrossingWave: public Wave
{
  public:
	enum State
	{
		NotStarted,
		Running,
		Finished,
	};

  public:
	// Ctor
	CrossingWave() noexcept;

	// Ctor
	CrossingWave (Wave* from, Wave* to, std::size_t transition_samples) noexcept;

	// Copy-assign:
	CrossingWave&
	operator= (CrossingWave const& other) noexcept;

	/**
	 * Return object state.
	 */
	State
	state() const noexcept;

	/**
	 * Start transitioning the two waves.
	 * Use advance() to move on with transition.
	 */
	void
	start() noexcept;

	/**
	 * Advance to next processing round.
	 */
	void
	advance (std::size_t samples) noexcept;

	/**
	 * Switch back to NotStarted state.
	 */
	void
	reset() noexcept;

	/**
	 * Returns function's sample.
	 */
	Sample
	operator() (Sample phase, Sample frequency, std::size_t sample) const noexcept override;

  private:
	State		_state				= NotStarted;
	Wave*		_wave_from			= nullptr;
	Wave*		_wave_to			= nullptr;
	std::size_t	_current_sample		= 0;
	std::size_t	_transition_samples	= 0;
};


inline CrossingWave::State
CrossingWave::state() const noexcept
{
	return _state;
}

} // namespace DSP
} // namespace Haruhi

#endif

