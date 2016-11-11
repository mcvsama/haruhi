/* vim:ts=4
 *
 * Copyleft 2008…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__UTILITY__LOG_SCALE_H__INCLUDED
#define HARUHI__UTILITY__LOG_SCALE_H__INCLUDED

// Standard:
#include <cstddef>
#include <type_traits>
#include <cmath>

// Haruhi:
#include <haruhi/config/all.h>


template<class pValue>
	class LogScale
	{
	  public:
		typedef pValue Value;

	  public:
		// Ctor
		explicit constexpr
		LogScale (Value bels);

		/**
		 * Unary negative.
		 */
		constexpr LogScale
		operator-() const;

		/**
		 * Return absolute value factor.
		 */
		constexpr Value
		factor() const;

	  private:
		Value _bels;
	};


static_assert (std::is_literal_type<LogScale<double>>::value, "LogScale<> must be a literal type");


template<class V>
	constexpr
	LogScale<V>::LogScale (Value bels):
		_bels (bels)
	{ }


template<class V>
	constexpr LogScale<V>
	LogScale<V>::operator-() const
	{
		return LogScale (-_bels);
	}


template<class V>
	constexpr typename LogScale<V>::Value
	LogScale<V>::factor() const
	{
		return std::pow (10.0, _bels / 2.0);
	}


/*
 * Global operators
 */


template<class FloatType, class BelValue>
	constexpr FloatType
	operator* (FloatType k, LogScale<BelValue> scale)
	{
		return k * scale.factor();
	}


template<class FloatType, class BelValue>
	constexpr FloatType
	operator+ (FloatType k, LogScale<BelValue> scale)
	{
		return k * scale;
	}


template<class FloatType, class BelValue>
	constexpr FloatType
	operator- (FloatType k, LogScale<BelValue> scale)
	{
		return k * -scale;
	}


/*
 * User-defined literals
 */


constexpr LogScale<long double>
operator"" _dB (long double db)
{
	return LogScale<long double> (db / 10.0);
}


constexpr LogScale<long double>
operator"" _dB (unsigned long long db)
{
	return LogScale<long double> (db / 10.0);
}

#endif

