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

#ifndef HARUHI__PLUGINS__YUKI__FILTER_IR_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__FILTER_IR_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/impulse_response.h>
#include <haruhi/utility/numeric.h>


namespace Yuki {

namespace DSP = Haruhi::DSP;
using Haruhi::Sample;

/*
 * Implementation of IIR filters described in
 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 *
 * Band pass/constant gain is not used directly as exactly the same
 * response has variable gain filter with limiter enabled.
 */
class FilterImpulseResponse: public DSP::ImpulseResponse<3, DSP::IIR>
{
  public:
	enum Type {
		LowPass		= 0,
		HighPass	= 1,
		BandPass	= 2,
		Notch		= 3,
		AllPass		= 4,
		Peaking		= 5,
		LowShelf	= 6,
		HighShelf	= 7,
	};

  public:
	FilterImpulseResponse (Type type = LowPass, Sample frequency = 0.0f, Sample resonance = 0.0f, Sample gain = 0.0f, Sample attenuation = 0.0f) noexcept;

	/**
	 * Return filter type.
	 */
	Type
	type() const noexcept;

	/**
	 * Set filter type.
	 */
	void
	set_type (Type type) noexcept;

	/**
	 * Return current center frequency.
	 */
	Sample
	frequency() const noexcept;

	/**
	 * Set center frequency.
	 * \param	frequency
	 *			Frequency. Value in range (0, 0.5).
	 */
	void
	set_frequency (Sample) noexcept;

	/**
	 * Return current resonance.
	 */
	Sample
	resonance() const noexcept;

	/**
	 * Set Q/resonance.
	 * Resonance will be set = 0.01 if it is less than it.
	 * \param	resonance
	 *			Q/resonance. Value in range (0, 1.0]
	 */
	void
	set_resonance (Sample resonance) noexcept;

	/**
	 * Return current gain.
	 */
	Sample
	gain() const noexcept;

	/**
	 * Set gain for Peaking and Shelving filters.
	 * \param	gain
	 *			Gain in decibels.
	 */
	void
	set_gain (Sample gain) noexcept;

	/**
	 * Get current attenuation.
	 */
	Sample
	attenuation() const noexcept;

	/**
	 * Set attenuation for filter.
	 */
	void
	set_attenuation (Sample attenuation) noexcept;

	/**
	 * Return true if attenuation limiter is enabled.
	 */
	bool
	limiter_enabled() const noexcept;

	/**
	 * Enables/disables automatic attenuation limiter.
	 */
	void
	set_limiter_enabled (bool enabled) noexcept;

	/*
	 * ImpulseResponse API
	 */

	Sample
	response (Sample frequency) const noexcept;

  private:
	void
	update() noexcept;

  private:
	Type	_type;
	Sample	_frequency;
	Sample	_resonance;
	Sample	_gain;
	Sample	_attenuation;
	bool	_limiter;
	// Helpers:
	bool	_dont_update;
};


inline FilterImpulseResponse::Type
FilterImpulseResponse::type() const noexcept
{
	return _type;
}


inline void
FilterImpulseResponse::set_type (Type type) noexcept
{
	if (_type != type)
	{
		_type = type;
		update();
	}
}


inline Sample
FilterImpulseResponse::frequency() const noexcept
{
	return _frequency;
}


inline void
FilterImpulseResponse::set_frequency (Sample frequency) noexcept
{
	// Limit frequency to 32Hz…23.99kHz for fs=48kHz
	frequency = bound (frequency, 0.0006666666f, 0.4997916666f);
	if (_frequency != frequency)
	{
		_frequency = frequency;
		update();
	}
}


inline Sample
FilterImpulseResponse::resonance() const noexcept
{
	return _resonance;
}


inline void
FilterImpulseResponse::set_resonance (Sample resonance) noexcept
{
	// Q must be greater than 0:
	resonance = std::max (0.01f, resonance);
	if (_resonance != resonance)
	{
		_resonance = resonance;
		update();
	}
}


inline Sample
FilterImpulseResponse::gain() const noexcept
{
	return _gain;
}


inline void
FilterImpulseResponse::set_gain (Sample gain) noexcept
{
	if (_gain != gain)
	{
		_gain = gain;
		update();
	}
}


inline Sample
FilterImpulseResponse::attenuation() const noexcept
{
	return _attenuation;
}


inline void
FilterImpulseResponse::set_attenuation (Sample attenuation) noexcept
{
	if (_attenuation != attenuation)
	{
		_attenuation = attenuation;
		update();
	}
}


inline bool
FilterImpulseResponse::limiter_enabled() const noexcept
{
	return _limiter;
}


inline void
FilterImpulseResponse::set_limiter_enabled (bool enabled) noexcept
{
	if (_limiter != enabled)
	{
		_limiter = enabled;
		update();
	}
}

} // namespace Yuki

#endif

