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

#ifndef HARUHI__CONTROLLER_PARAM_H__INCLUDED
#define HARUHI__CONTROLLER_PARAM_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/saveable_state.h>


namespace Haruhi {

/**
 * Represents integer parameter that can be controlled by
 * Haruhi::Controller. Also can contain information about
 * parameter smoothing.
 */
class ControllerParam: public SaveableState
{
  public:
	ControllerParam();

	ControllerParam (int minimum, int maximum, int denominator, int default_value);

	ControllerParam (ControllerParam const& other) { *this = other; }

	ControllerParam&
	operator= (ControllerParam const& other);

	int
	minimum() const { return _minimum; }

	int
	maximum() const { return _maximum; }

	int
	denominator() const { return _denominator; }

	int
	default_value() const { return _default_value; }

	int
	get() const { return atomic (_parameter); }

	void
	set (int value) { atomic (_parameter) = value; }

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
	to_f() const { return 1.0f * atomic (_parameter) / _denominator; }

	/**
	 * Resets parameter to default value.
	 */
	void
	reset() { set (_default_value); }

	volatile int*
	parameter() { return &_parameter; }

	volatile int*
	smoothing_parameter() { return &_smoothing_parameter; }

	/*
	 * SaveableState API
	 */

    void
    save_state (QDomElement&) const;

    void
    load_state (QDomElement const&);

  private:
	int				_minimum;
	int				_maximum;
	int				_denominator;
	int				_default_value;
	bool			_smoothing_enabled;

	volatile int	_parameter;
	volatile int	_smoothing_parameter;
};

} // namespace Haruhi

#endif

