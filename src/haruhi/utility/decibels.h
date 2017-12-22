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

#ifndef HARUHI__UTILITY__DECIBELS_H__INCLUDED
#define HARUHI__UTILITY__DECIBELS_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>

// Haruhi:
#include <haruhi/config/all.h>


class Decibels
{
  public:
	constexpr explicit
	Decibels (float decibels) noexcept;

	constexpr
	Decibels (Decibels const& other) noexcept;

	/**
	 * Same as to_f().
	 */
	constexpr explicit
	operator float() noexcept;

	/**
	 * Negate decibels value.
	 */
	constexpr Decibels
	operator-() noexcept;

	/**
	 * Return multiplication factor,
	 * eg. for value -3_dB return ~0.74.
	 */
	constexpr float
	factor() const noexcept;

	/**
	 * Return number of decibels as a float value,
	 * eg. -3_dB will be converted to -3.0f.
	 */
	constexpr float
	to_f() const noexcept;

  public:
	/**
	 * Create Decibels value from multiplication factor, which
	 * must be >= 0.0f.
	 */
	// FIXME: GCC bug <http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53473> - remove "(false)" when this is fixed.
	static constexpr Decibels
	from_factor (float multiplication_factor) noexcept;

  private:
	float _decibels;
};


constexpr
Decibels::Decibels (float decibels) noexcept:
	_decibels (decibels)
{ }


constexpr
Decibels::Decibels (Decibels const& other) noexcept:
	_decibels (other._decibels)
{ }


constexpr
Decibels::operator float() noexcept
{
	return to_f();
}


constexpr Decibels
Decibels::operator-() noexcept
{
	return Decibels (1.0 / _decibels);
}


constexpr float
Decibels::factor() const noexcept
{
	return std::pow (10.0f, _decibels / 10.0f);
}


constexpr float
Decibels::to_f() const noexcept
{
	return _decibels;
}


// FIXME: GCC bug <http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53473> - remove "(false)" when this is fixed.
constexpr Decibels
Decibels::from_factor (float multiplication_factor) noexcept
{
	return Decibels (10.0f * std::log10 (multiplication_factor));
}


/*
 * Global functions
 */


constexpr Decibels
operator"" _dB (long double decibels)
{
	return Decibels (decibels);
}


constexpr Decibels
operator"" _dB (unsigned long long decibels)
{
	return Decibels (static_cast<float> (decibels));
}


/**
 * Adjust numeric value by @decibels decibels.
 */
template<class Numeric>
	constexpr Numeric
	operator+ (Numeric value, Decibels decibels) noexcept
	{
		return value * decibels.factor();
	}


/**
 * Adjust numeric value by -@decibels decibels.
 */
template<class Numeric>
	constexpr Numeric
	operator- (Numeric value, Decibels decibels) noexcept
	{
		return value * (-decibels).factor();
	}

#endif

