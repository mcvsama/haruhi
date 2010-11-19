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
 * Extends Param with smoothing parameter.
 */
class ControllerParam:
	public Param<int>,
	public SaveableState
{
  public:
	ControllerParam();

	ControllerParam (int minimum, int maximum, int default_value, int denominator = 1);

	ControllerParam (ControllerParam const& other) { *this = other; }

	ControllerParam&
	operator= (ControllerParam const& other);

	int
	denominator() const { return _denominator; }

	int
	smoothing() const { return atomic (_smoothing_parameter); }

	void
	set_smoothing (int smoothing_ms)
	{
		_smoothing_enabled = true;
		atomic (_smoothing_parameter) = smoothing_ms;
	}

	bool
	smoothing_enabled() const { return _smoothing_enabled; }

	void
	set_smoothing_enabled (bool setting) { _smoothing_enabled = setting; }

	/**
	 * Returns value divided by denominator and casted to float.
	 */
	float
	to_f() const { return 1.0f * get() / _denominator; }

	volatile int*
	smoothing_parameter() { return &_smoothing_parameter; }

	/**
	 * Calls Param<int>::sanitize().
	 * Additionally sanitizes smoothing parameter.
	 */
	void
	sanitize();

	/*
	 * SaveableState API
	 */

    void
    save_state (QDomElement&) const;

    void
    load_state (QDomElement const&);

  private:
	int						_denominator;
	bool					_smoothing_enabled;
	int volatile mutable	_smoothing_parameter; // Mutable for atomic().
};

} // namespace Haruhi

#endif

