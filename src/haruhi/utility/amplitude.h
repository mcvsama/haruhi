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

#ifndef HARUHI__UTILITY__AMPLITUDE_H__INCLUDED
#define HARUHI__UTILITY__AMPLITUDE_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>


class Amplitude
{
  public:
	explicit Amplitude (float multiplication_factor) noexcept;

	bool
	operator< (Amplitude const& other) const noexcept;

	bool
	operator<= (Amplitude const& other) const noexcept;

	bool
	operator> (Amplitude const& other) const noexcept;

	bool
	operator>= (Amplitude const& other) const noexcept;

	Amplitude
	operator* (Amplitude const& other) const noexcept;

	Amplitude&
	operator*= (Amplitude const& other) noexcept;

	/**
	 * Return amplitude multiplication factor (value).
	 */
	float
	factor() const noexcept;

	/**
	 * Set amplitude from multiplication factor (value).
	 */
	void
	set_factor (float multiplication_factor) noexcept;

	/**
	 * Return amplitude represented in number of decibels.
	 */
	float
	decibels() const noexcept;

	/**
	 * Set amplitude from number of decibels.
	 */
	void
	set_decibels (float decibels) noexcept;

  private:
	float _factor;
};


inline
Amplitude::Amplitude (float multiplication_factor) noexcept:
	_factor (multiplication_factor)
{ }


inline bool
Amplitude::operator< (Amplitude const& other) const noexcept
{
	return _factor < other._factor;
}


inline bool
Amplitude::operator<= (Amplitude const& other) const noexcept
{
	return _factor <= other._factor;
}


inline bool
Amplitude::operator> (Amplitude const& other) const noexcept
{
	return _factor > other._factor;
}


inline bool
Amplitude::operator>= (Amplitude const& other) const noexcept
{
	return _factor >= other._factor;
}


inline Amplitude
Amplitude::operator* (Amplitude const& other) const noexcept
{
	return Amplitude (_factor * other._factor);
}


inline Amplitude&
Amplitude::operator*= (Amplitude const& other) noexcept
{
	_factor *= other._factor;
	return *this;
}


inline float
Amplitude::factor() const noexcept
{
	return _factor;
}


inline void
Amplitude::set_factor (float multiplication_factor) noexcept
{
	_factor = multiplication_factor;
}


inline float
Amplitude::decibels() const noexcept
{
	return 20.0f * std::log10 (_factor);
}


inline void
Amplitude::set_decibels (float decibels) noexcept
{
	_factor = std::pow (10.0f, decibels / 20.0f);
}


/*
 * Global functions.
 */


inline Haruhi::Sample
operator* (Haruhi::Sample sample, Amplitude amplitude) noexcept
{
	return sample * amplitude.factor();
}


inline Haruhi::Sample
operator* (Amplitude amplitude, Haruhi::Sample sample) noexcept
{
	return sample * amplitude.factor();
}


#if 0
inline Amplitude
operator"" _dB (long double db)
{
	Amplitude a;
	a.set_decibels (db);
	return a;
}
#endif

#endif

