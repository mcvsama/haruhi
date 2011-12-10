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

#ifndef HARUHI__LIB__CONTROLLER_PARAM_H__INCLUDED
#define HARUHI__LIB__CONTROLLER_PARAM_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/utility/atomic.h>
#include <haruhi/lib/param.h>


namespace Haruhi {

/**
 * Param that can be controlled by Haruhi::Controller.
 */
class ControllerParam: public Param<int>
{
  public:
	ControllerParam (const char* name = "");

	ControllerParam (int minimum, int maximum, int default_value, int denominator, const char* name);

	ControllerParam (ControllerParam const& other);

	ControllerParam&
	operator= (ControllerParam const& other);

	int
	denominator() const;

	/**
	 * Return value divided by denominator and casted to float.
	 */
	float
	to_f() const;

  private:
	int		_denominator;
	float	_1_div_denominator;
};


inline
ControllerParam::ControllerParam (const char* name):
	Param (name),
	_denominator (1),
	_1_div_denominator (1.0f / _denominator)
{ }


inline
ControllerParam::ControllerParam (int minimum, int maximum, int default_value, int denominator, const char* name):
	Param (minimum, maximum, default_value, name),
	_denominator (denominator),
	_1_div_denominator (1.0f / _denominator)
{ }


inline
ControllerParam::ControllerParam (ControllerParam const& other):
	Param (other)
{
	_denominator = other._denominator;
	_1_div_denominator = other._1_div_denominator;
}


inline ControllerParam&
ControllerParam::operator= (ControllerParam const& other)
{
	Param<int>::operator= (other);
	_denominator = other._denominator;
	_1_div_denominator = other._1_div_denominator;
	return *this;
}


inline int
ControllerParam::denominator() const
{
	return _denominator;
}


inline float
ControllerParam::to_f() const
{
	return _1_div_denominator * get();
}

} // namespace Haruhi

#endif

