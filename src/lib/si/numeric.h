/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef SI__NUMERIC_H__INCLUDED
#define SI__NUMERIC_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>


namespace SI {

/**
 * For floats.
 * \param	n - dividend
 * \param	d - divisor
 */
template<class Number>
	inline constexpr Number
	floored_mod (Number n, typename std::enable_if<std::is_floating_point<Number>::value, Number>::type d)
	{
		return n - (d * std::floor (n / d));
	}


/**
 * For integral types.
 * \param	n - dividend
 * \param	d - divisor
 */
template<class Number>
	inline constexpr Number
	floored_mod (Number n, typename std::enable_if<std::is_integral<Number>::value, Number>::type d)
	{
		return (n % d) >= 0 ? (n % d) : (n % d) + std::abs (d);
	}


template<class Number>
	inline constexpr Number
	floored_mod (Number n, Number min, Number max)
	{
		return floored_mod (n - min, max - min) + min;
	}

} // namespace SI

#endif

