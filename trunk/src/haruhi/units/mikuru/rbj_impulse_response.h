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

#ifndef HARUHI__UNITS__MIKURU__RBJ_IIR_H__INCLUDED
#define HARUHI__UNITS__MIKURU__RBJ_IIR_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Haruhi:
#include <haruhi/core/audio.h>
#include <haruhi/dsp/impulse_response.h>
#include <haruhi/haruhi.h>


namespace MikuruPrivate {

using Core::Sample;


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

	Type
	type() const { return _type; }

	void
	set_type (Type type);

	/**
	 * Sets center frequency.
	 *
	 * \param	frequency
	 * 			Frequency. Value in range (0, 0.5).
	 */
	void
	set_frequency (Sample);

	Sample
	frequency() const { return _frequency; }

	/**
	 * Sets Q/resonance. Resonance will be set = 0.01 if it is less than it.
	 *
	 * \param	resonance
	 * 			Q/resonance. Value in range (0, 1.0]
	 */
	void
	set_resonance (Sample resonance);

	Sample
	resonance() const { return _resonance; }

	/**
	 * Sets gain for Peaking and Shelving filters.
	 *
	 * \param	gain
	 * 			Gain in decibels.
	 */
	void
	set_gain (Sample gain);

	Sample
	gain() const { return _gain; }

	/**
	 * Sets attenuation for filter.
	 */
	void
	set_attenuation (Sample attenuation);

	Sample
	attenuation() const { return _attenuation; }

	/**
	 * Enables/disables automatic attenuation limiter.
	 */
	void
	set_limiter (bool enabled);

	bool
	limiter() const { return _limiter; }

	int
	size() const { return 3; }

	/**
	 * ImpulseResponse::response()
	 */
	Sample
	response (Sample frequency) const;

	/**
	 * IIR::a();
	 */
	Sample*
	a() { return _a; }

	/**
	 * IIR::b();
	 */
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

} // namespace MikuruPrivate

#endif

