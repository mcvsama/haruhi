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
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/lib/param.h>


namespace Haruhi {

/**
 * Param that can be controlled by Haruhi::Controller.
 */
class ControllerParam:
	public Param<int>,
	public SaveableState
{
  public:
	/**
	 * Inline for performance reasons.
	 */
	ControllerParam():
		_denominator (1)
	{ }

	/**
	 * Inline for performance reasons.
	 */
	ControllerParam (int minimum, int maximum, int default_value, int denominator = 1):
		Param<int> (minimum, maximum, default_value),
		_denominator (denominator)
	{ }

	ControllerParam (ControllerParam const& other) { *this = other; }

	/**
	 * Inline for performance reasons.
	 */
	ControllerParam&
	operator= (ControllerParam const& other)
	{
		Param<int>::operator= (other);
		_denominator = other._denominator;
		return *this;
	}

	int
	denominator() const { return _denominator; }

	/**
	 * Returns value divided by denominator and casted to float.
	 */
	float
	to_f() const { return 1.0f * get() / _denominator; }

	/*
	 * SaveableState API
	 */

    void
    save_state (QDomElement&) const;

    void
    load_state (QDomElement const&);

  private:
	int	_denominator;
};

} // namespace Haruhi

#endif

