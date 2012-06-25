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

#ifndef HARUHI__UTILITY__SECONDS_H__INCLUDED
#define HARUHI__UTILITY__SECONDS_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>

// Haruhi:
#include <haruhi/config/all.h>


class Hertz;

class Seconds
{
  public:
	constexpr explicit
	Seconds (float sec) noexcept;

	constexpr
	Seconds (Seconds const& other) noexcept;

	/**
	 * Same as to_f().
	 */
	constexpr explicit
	operator float() noexcept;

	/**
	 * Return number of seconds as a float value,
	 * eg. 100 s will be converted to 100.0f.
	 */
	constexpr float
	to_f() const noexcept;

	/**
	 * Return value as milliseconds.
	 */
	constexpr float
	milliseconds() const noexcept;

	/**
	 * Return value as microseconds.
	 */
	constexpr float
	microseconds() const noexcept;

	constexpr bool
	operator== (Seconds const& other) const noexcept;

	template<class Numeric>
		constexpr bool
		operator== (Numeric other) const noexcept;

	template<class Numeric>
		constexpr bool
		operator> (Numeric other) const noexcept;

	template<class Numeric>
		constexpr bool
		operator>= (Numeric other) const noexcept;

	template<class Numeric>
		constexpr bool
		operator< (Numeric other) const noexcept;

	template<class Numeric>
		constexpr bool
		operator<= (Numeric other) const noexcept;

	Seconds&
	operator*= (float factor) noexcept;

	Seconds&
	operator/= (float factor) noexcept;

	float
	operator/= (Seconds const& other) noexcept;

	constexpr Seconds
	operator* (float factor) const noexcept;

	constexpr float
	operator* (Hertz hertz) const noexcept;

	constexpr Seconds
	operator/ (float factor) const noexcept;

	constexpr float
	operator/ (Seconds const& other) const noexcept;

  private:
	float _seconds;
};


#include "hertz.h"


inline constexpr
Seconds::Seconds (float seconds) noexcept:
	_seconds (seconds)
{ }


inline constexpr
Seconds::Seconds (Seconds const& other) noexcept:
	_seconds (other._seconds)
{ }


inline constexpr
Seconds::operator float() noexcept
{
	return to_f();
}


inline constexpr float
Seconds::to_f() const noexcept
{
	return _seconds;
}


inline constexpr float
Seconds::milliseconds() const noexcept
{
	return _seconds * 1e3f;
}


inline constexpr float
Seconds::microseconds() const noexcept
{
	return _seconds * 1e6f;
}


inline constexpr bool
Seconds::operator== (Seconds const& other) const noexcept
{
	return _seconds == other._seconds;
}


template<class Numeric>
	inline constexpr bool
	Seconds::operator== (Numeric other) const noexcept
	{
		return _seconds == other;
	}


template<class Numeric>
	inline constexpr bool
	Seconds::operator> (Numeric other) const noexcept
	{
		return _seconds > other;
	}


template<class Numeric>
	inline constexpr bool
	Seconds::operator>= (Numeric other) const noexcept
	{
		return _seconds >= other;
	}


template<class Numeric>
	inline constexpr bool
	Seconds::operator< (Numeric other) const noexcept
	{
		return _seconds < other;
	}


template<class Numeric>
	inline constexpr bool
	Seconds::operator<= (Numeric other) const noexcept
	{
		return _seconds <= other;
	}


inline Seconds&
Seconds::operator*= (float factor) noexcept
{
	_seconds *= factor;
	return *this;
}


inline Seconds&
Seconds::operator/= (float factor) noexcept
{
	_seconds /= factor;
	return *this;
}


inline float
Seconds::operator/= (Seconds const& other) noexcept
{
	return _seconds / other._seconds;
}


inline constexpr Seconds
Seconds::operator* (float factor) const noexcept
{
	return Seconds (_seconds * factor);
}


inline constexpr float
Seconds::operator* (Hertz hertz) const noexcept
{
	return _seconds * static_cast<float> (hertz);
}


inline constexpr Seconds
Seconds::operator/ (float factor) const noexcept
{
	return Seconds (_seconds / factor);
}


inline constexpr float
Seconds::operator/ (Seconds const& other) const noexcept
{
	return _seconds / other._seconds;
}


/*
 * Global functions
 */


inline constexpr Seconds
operator"" _s (long double seconds)
{
	return Seconds (seconds);
}


inline constexpr Seconds
operator"" _s (unsigned long long seconds)
{
	return Seconds (static_cast<float> (seconds));
}


inline constexpr Seconds
operator"" _ms (long double ms)
{
	return Seconds (1e-3 * ms);
}


inline constexpr Seconds
operator"" _ms (unsigned long long ms)
{
	return Seconds (1e-3 * static_cast<float> (ms));
}


inline constexpr Seconds
operator"" _us (long double us)
{
	return Seconds (1e-6 * us);
}


inline constexpr Seconds
operator"" _us (unsigned long long us)
{
	return Seconds (1e-6 * static_cast<float> (us));
}


/**
 * Same as Seconds.operator* (float).
 */
inline constexpr Seconds
operator* (float value, Seconds seconds) noexcept
{
	return seconds * value;
}


/**
 * Return inverse value in Hertz.
 */
inline constexpr Hertz
operator/ (float value, Seconds seconds) noexcept
{
	return Hertz (value / static_cast<float> (seconds));
}


template<class Numeric>
	inline constexpr bool
	operator> (Numeric other, Seconds seconds) noexcept
	{
		return other > seconds;
	}


template<class Numeric>
	inline constexpr bool
	operator>= (Numeric other, Seconds seconds) noexcept
	{
		return other >= seconds;
	}


template<class Numeric>
	inline constexpr bool
	operator< (Numeric other, Seconds seconds) noexcept
	{
		return other < seconds;
	}


template<class Numeric>
	inline constexpr bool
	operator<= (Numeric other, Seconds seconds) noexcept
	{
		return other <= seconds;
	}

#endif

