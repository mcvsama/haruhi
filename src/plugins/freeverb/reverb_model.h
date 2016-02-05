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

#ifndef HARUHI__PLUGINS__FREEVERB__MODEL_H__INCLUDED
#define HARUHI__PLUGINS__FREEVERB__MODEL_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "comb_filter.h"
#include "allpass_filter.h"


namespace Freeverb {

class ReverbModel
{
	static constexpr int	kNumCombs		= 8;
	static constexpr int	kNumAllpasses	= 4;
	static constexpr float	kFixedGain		= 0.015;
	static constexpr float	kScaleDamp		= 0.4;
	static constexpr float	kScaleRoom		= 0.28;
	static constexpr float	kOffsetRoom		= 0.7;
	static constexpr float	kInitialRoom	= 0.5;
	static constexpr float	kInitialDamp	= 0.5;
	static constexpr float	kInitialWidth	= 1.0;
	static constexpr float	kInitialMode	= 0.0;
	static constexpr int	kStereoSpread	= 23;

	// Original comment from Freeverb implementation:
	//   These values assume 44.1KHz sample rate
	//   they will probably be OK for 48KHz sample rate
	//   but would need scaling for 96KHz (or other) sample rates.
	//   The values were obtained by listening tests.
	// Note: those values are scaled for sample rate in this program.
	static constexpr std::array<int, kNumCombs>		kCombTuningsL = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
	static constexpr std::array<int, kNumCombs>		kCombTuningsR = get_offset_array (kCombTuningsL, kStereoSpread);
	static constexpr std::array<int, kNumAllpasses>	kAllpassTuningsL = { 556, 441, 341, 255 };
	static constexpr std::array<int, kNumAllpasses>	kAllpassTuningsR = get_offset_array (kAllpassTuningsL, kStereoSpread);

  public:
	enum class Mode
	{
		Normal,
		Freeze,
	};

  public:
	// Ctor
	ReverbModel (Frequency sample_rate);

	/**
	 * Clear buffers.
	 */
	void
	clear();

	/**
	 * Process N samples.
	 * TODO convert to use std::array_view<>
	 */
	void
	process (Haruhi::Sample* inputL, Haruhi::Sample* inputR, Haruhi::Sample* outputL, Haruhi::Sample* outputR, size_t samples);

	/**
	 * Set the room-size parameter.
	 */
	void
	set_room_size (float value);

	/**
	 * Set the damping parameter.
	 */
	void
	set_damping (float value);

	/**
	 * Set the width parameter.
	 */
	void
	set_width (float value);

	/**
	 * Set mode.
	 */
	void
	set_mode (Mode mode);

	/**
	 * Notifies about new sample rate, so model can recompute
	 * its internal values for the new value.
	 */
	void
	set_sample_rate (Frequency sample_rate);

  private:
	/**
	 * Resize buffers according to new sample rate, since
	 * filters' params depend on it.
	 */
	void
	scale_buffers (Frequency sample_rate);

	/**
	 * Recomputes internal values after parameter change.
	 */
	void
	update();

  private:
	float	_gain		= 0.0;
	float	_room_size	= 0.0;
	float	_room_size1	= 0.0;
	float	_damp		= 0.0;
	float	_damp1		= 0.0;
	float	_wet1		= 0.0;
	float	_wet2		= 0.0;
	float	_width		= 0.0;
	Mode	_mode		= Mode::Normal;

	// Comb filters:
	std::array<CombFilter, kNumCombs> _combL;
	std::array<CombFilter, kNumCombs> _combR;

	// Allpass filters:
	std::array<AllpassFilter, kNumAllpasses> _allpassL;
	std::array<AllpassFilter, kNumAllpasses> _allpassR;
};

} // namespace Freeverb

#endif

