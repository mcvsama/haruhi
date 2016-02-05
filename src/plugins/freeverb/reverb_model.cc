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

// Local:
#include "reverb_model.h"


namespace Freeverb {

constexpr int	ReverbModel::kNumCombs;
constexpr int	ReverbModel::kNumAllpasses;
constexpr float	ReverbModel::kFixedGain;
constexpr float	ReverbModel::kScaleDamp;
constexpr float	ReverbModel::kScaleRoom;
constexpr float	ReverbModel::kOffsetRoom;
constexpr float	ReverbModel::kInitialRoom;
constexpr float	ReverbModel::kInitialDamp;
constexpr float	ReverbModel::kInitialWidth;
constexpr float	ReverbModel::kInitialMode;
constexpr int	ReverbModel::kStereoSpread;

constexpr std::array<int, ReverbModel::kNumCombs>		ReverbModel::kCombTuningsL;
constexpr std::array<int, ReverbModel::kNumCombs>		ReverbModel::kCombTuningsR;
constexpr std::array<int, ReverbModel::kNumAllpasses>	ReverbModel::kAllpassTuningsL;
constexpr std::array<int, ReverbModel::kNumAllpasses>	ReverbModel::kAllpassTuningsR;


ReverbModel::ReverbModel (Frequency sample_rate)
{
	scale_buffers (sample_rate);

	// Set default values:
	for (auto& filter: _allpassL)
		filter.set_feedback (0.5);

	for (auto& filter: _allpassR)
		filter.set_feedback (0.5);

	set_room_size (kInitialRoom);
	set_damping (kInitialDamp);
	set_width (kInitialWidth);
	set_mode (Mode::Normal);

	// Initial forced update:
	update();
	clear();
}


void
ReverbModel::clear()
{
	if (_mode == Mode::Freeze)
		return;

	for (auto& filter: _combL)
		filter.clear();

	for (auto& filter: _combR)
		filter.clear();

	for (auto& filter: _allpassL)
		filter.clear();

	for (auto& filter: _allpassR)
		filter.clear();
}


void
ReverbModel::process (Haruhi::Sample* inputL, Haruhi::Sample* inputR, Haruhi::Sample* outputL, Haruhi::Sample* outputR, size_t samples)
{
	while (samples-- > 0)
	{
		float outL = 0;
		float outR = 0;
		float input = (*inputL + *inputR) * _gain;

		// Accumulate comb filters in parallel:
		for (auto& filter: _combL)
			outL += filter.process (input);

		for (auto& filter: _combR)
			outR += filter.process (input);

		// Feed through allpasses in series:
		for (auto& filter: _allpassL)
			outL = filter.process (outL);

		for (auto& filter: _allpassR)
			outR = filter.process (outR);

		// Write output:
		*outputL = outL * _wet1 + outR * _wet2;
		*outputR = outR * _wet1 + outL * _wet2;

		// Increment sample pointers:
		inputL += 1;
		inputR += 1;

		outputL += 1;
		outputR += 1;
	}
}


void
ReverbModel::set_room_size (float value)
{
	float s = FastPow::pow (value, 0.5) * kScaleRoom + kOffsetRoom;

	if (_room_size != s)
	{
		_room_size = s;
		update();
	}
}


void
ReverbModel::set_damping (float value)
{
	if (_damp != value * kScaleDamp)
	{
		_damp = value * kScaleDamp;
		update();
	}
}


void
ReverbModel::set_width (float value)
{
	if (_width != value)
	{
		_width = value;
		update();
	}
}


void
ReverbModel::set_mode (Mode mode)
{
	if (_mode != mode)
	{
		_mode = mode;
		update();
	}
}


void
ReverbModel::set_sample_rate (Frequency sample_rate)
{
	scale_buffers (sample_rate);
	update();
}


void
ReverbModel::scale_buffers (Frequency sample_rate)
{
	// Original values are obtained for 44.1kHz, need to scale them.
	float scale_factor = sample_rate / 44100.0_Hz;

	for (std::size_t i = 0; i < _combL.size(); ++i)
		_combL[i].set_buffer_size (scale_factor * kCombTuningsL[i]);

	for (std::size_t i = 0; i < _combR.size(); ++i)
		_combR[i].set_buffer_size (scale_factor * kCombTuningsR[i]);

	for (std::size_t i = 0; i < _allpassL.size(); ++i)
		_allpassL[i].set_buffer_size (scale_factor * kAllpassTuningsL[i]);

	for (std::size_t i = 0; i < _allpassR.size(); ++i)
		_allpassR[i].set_buffer_size (scale_factor * kAllpassTuningsR[i]);
}


void
ReverbModel::update()
{
	float wet = 3.0;
	_wet1 = wet * (_width / 2.0 + 0.5);
	_wet2 = wet * (1.0 - _width) / 2.0;

	switch (_mode)
	{
		case Mode::Freeze:
			_room_size1 = 1.0;
			_damp1 = 0.0;
			_gain = 0.0;
			break;

		case Mode::Normal:
			_room_size1 = _room_size;
			_damp1 = _damp;
			_gain = kFixedGain;
			break;
	}

	for (auto& filter: _combL)
	{
		filter.set_feedback (_room_size1);
		filter.set_damping (_damp1);
	}

	for (auto& filter: _combR)
	{
		filter.set_feedback (_room_size1);
		filter.set_damping (_damp1);
	}
}

} // namespace Freeverb

