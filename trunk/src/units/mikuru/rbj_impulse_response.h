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

#ifndef HARUHI__UNITS__MIKURU__RBJ_IIR_H__INCLUDED
#define HARUHI__UNITS__MIKURU__RBJ_IIR_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/impulse_response.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "types.h"


namespace MikuruPrivate {

namespace DSP = Haruhi::DSP;

/*
 * Implementation of IIR filters described in
 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 *
 * Band pass/constant gain is not used directly as exactly the same
 * response has variable gain filter with limiter enabled.
 */
class RBJImpulseResponse: public DSP::ImpulseResponse
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
	RBJImpulseResponse (Type type, Sample frequency, Sample resonance, Sample gain, Sample attenuation);

	inline void
	set_type (Type type);

	Type
	type() const { return _type; }

	/**
	 * Sets center frequency.
	 *
	 * \param	frequency
	 * 			Frequency. Value in range (0, 0.5).
	 */
	inline void
	set_frequency (Sample);

	Sample
	frequency() const { return _frequency; }

	/**
	 * Sets Q/resonance. Resonance will be set = 0.01 if it is less than it.
	 *
	 * \param	resonance
	 * 			Q/resonance. Value in range (0, 1.0]
	 */
	inline void
	set_resonance (Sample resonance);

	Sample
	resonance() const { return _resonance; }

	/**
	 * Sets gain for Peaking and Shelving filters.
	 *
	 * \param	gain
	 * 			Gain in decibels.
	 */
	inline void
	set_gain (Sample gain);

	Sample
	gain() const { return _gain; }

	/**
	 * Sets attenuation for filter.
	 */
	inline void
	set_attenuation (Sample attenuation);

	Sample
	attenuation() const { return _attenuation; }

	/**
	 * Enables/disables automatic attenuation limiter.
	 */
	inline void
	set_limiter (bool enabled);

	bool
	limiter() const { return _limiter; }

	/*
	 * ImpulseResponse API
	 */

	int
	size() const { return 3; }

	Sample
	response (Sample frequency) const;

	Sample*
	a() { return _a; }

	Sample*
	b() { return _b; }

  private:
	void
	update();

  private:
	Type	_type;
	Sample	_frequency;
	Sample	_resonance;
	Sample	_gain;
	Sample	_attenuation;
	bool	_limiter;
	// Filter coefficients:
	Sample	_a[3];
	Sample	_b[3];
	// Helpers:
	bool	_dont_update;
};

/*
 * Inline methods
 */

void
RBJImpulseResponse::set_type (Type type)
{
	if (_type != type)
	{
		_type = type;
		update();
	}
}


void
RBJImpulseResponse::set_frequency (Sample frequency)
{
	// Limit frequency to 32Hz…23.99kHz for fs=48kHz
	frequency = bound (frequency, 0.0006666666f, 0.4997916666f);
	if (_frequency != frequency)
	{
		_frequency = frequency;
		update();
	}
}


void
RBJImpulseResponse::set_resonance (Sample resonance)
{
	// Q must be greater than 0:
	resonance = std::max (0.01f, resonance);
	if (_resonance != resonance)
	{
		_resonance = resonance;
		update();
	}
}


void
RBJImpulseResponse::set_gain (Sample gain)
{
	if (_gain != gain)
	{
		_gain = gain;
		update();
	}
}


void
RBJImpulseResponse::set_attenuation (Sample attenuation)
{
	if (_attenuation != attenuation)
	{
		_attenuation = attenuation;
		update();
	}
}


void
RBJImpulseResponse::set_limiter (bool enabled)
{
	if (_limiter != enabled)
	{
		_limiter = enabled;
		update();
	}
}

} // namespace MikuruPrivate

#endif

