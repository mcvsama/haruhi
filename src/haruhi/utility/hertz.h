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

#ifndef HARUHI__UTILITY__HERTZ_H__INCLUDED
#define HARUHI__UTILITY__HERTZ_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>

// Haruhi:
#include <haruhi/config/all.h>


class Seconds;

class Hertz
{
  public:
	constexpr explicit
	Hertz (float hz) noexcept;

	constexpr
	Hertz (Hertz const& other) noexcept;

	/**
	 * Same as to_f().
	 */
	constexpr explicit
	operator float() noexcept;

	/**
	 * Return number of hertz as a float value,
	 * eg. 100 Hz will be converted to 100.0f.
	 */
	constexpr float
	to_f() const noexcept;

	constexpr bool
	operator< (Hertz const& other) noexcept;

	Hertz&
	operator*= (float factor) noexcept;

	Hertz&
	operator/= (float factor) noexcept;

	float
	operator/= (Hertz const& other) noexcept;

	constexpr Hertz
	operator* (float factor) const noexcept;

	constexpr float
	operator* (Seconds seconds) const noexcept;

	constexpr Hertz
	operator/ (float factor) const noexcept;

	constexpr float
	operator/ (Hertz const& other) const noexcept;

  private:
	float _hz;
};


#include "seconds.h"


inline constexpr
Hertz::Hertz (float hz) noexcept:
	_hz (hz)
{ }


inline constexpr
Hertz::Hertz (Hertz const& other) noexcept:
	_hz (other._hz)
{ }


inline constexpr
Hertz::operator float() noexcept
{
	return to_f();
}


inline constexpr float
Hertz::to_f() const noexcept
{
	return _hz;
}


inline constexpr bool
Hertz::operator< (Hertz const& other) noexcept
{
	return _hz < other._hz;
}


inline Hertz&
Hertz::operator*= (float factor) noexcept
{
	_hz *= factor;
	return *this;
}


inline Hertz&
Hertz::operator/= (float factor) noexcept
{
	_hz /= factor;
	return *this;
}


inline float
Hertz::operator/= (Hertz const& other) noexcept
{
	return _hz / other._hz;
}


inline constexpr Hertz
Hertz::operator* (float factor) const noexcept
{
	return Hertz (_hz * factor);
}


inline constexpr float
Hertz::operator* (Seconds seconds) const noexcept
{
	return _hz * static_cast<float> (seconds);
}


inline constexpr Hertz
Hertz::operator/ (float factor) const noexcept
{
	return Hertz (_hz / factor);
}


inline constexpr float
Hertz::operator/ (Hertz const& other) const noexcept
{
	return _hz / other._hz;
}


/*
 * Global functions
 */


inline constexpr Hertz
operator"" _Hz (long double hz)
{
	return Hertz (hz);
}


inline constexpr Hertz
operator"" _Hz (unsigned long long hz)
{
	return Hertz (static_cast<float> (hz));
}


inline constexpr Hertz
operator"" _kHz (long double khz)
{
	return Hertz (1e3 * khz);
}


inline constexpr Hertz
operator"" _kHz (unsigned long long khz)
{
	return Hertz (1e3 * static_cast<float> (khz));
}


/**
 * Same as Hertz.operator* (float).
 */
inline constexpr Hertz
operator* (float value, Hertz hertz) noexcept
{
	return hertz * value;
}


/**
 * Return inverse value in Seconds.
 */
inline constexpr Seconds
operator/ (float value, Hertz hertz) noexcept
{
	return Seconds (value / static_cast<float> (hertz));
}

#endif

