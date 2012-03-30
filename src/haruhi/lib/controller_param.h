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

#ifndef HARUHI__LIB__CONTROLLER_PARAM_H__INCLUDED
#define HARUHI__LIB__CONTROLLER_PARAM_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/lib/param.h>


namespace Haruhi {

/**
 * Param that can be controlled by Haruhi::Controller.
 * Also contains useful information for some controller widgets,
 * eg. min/max shown value, number of decimal digits shown, etc.
 */
class ControllerParam: public Param<int>
{
  public:
	/**
	 * Proxy configuration.
	 * Applies curve and limits to input data.
	 */
	class Adapter
	{
	  public:
		Adapter (int limit_min, int limit_max) noexcept;

		/**
		 * Applies forward transform. Takes input value,
		 * returns curved and limited value.
		 */
		int
		forward (int in) const noexcept;

		/**
		 * Applies reverse transform (inverse of what forward() does).
		 */
		int
		reverse (int in) const noexcept;

		/**
		 * Denormalize input value from range [0.0, 1.0] to
		 * range specified by the limits.
		 */
		int
		denormalize (float in) const noexcept;

		/**
		 * The same as forward(), but takes input value normalized
		 * into range [0.0, 1.0].
		 */
		int
		forward_normalized (float in) const noexcept;

	  private:
		int
		encurve (int in) const noexcept;

		int
		decurve (int in) const noexcept;

	  public:
		float	curve;
		int		hard_limit_min;
		int		hard_limit_max;
		int		user_limit_min;
		int		user_limit_max;
	};

  public:
	explicit ControllerParam (const char* name = "") noexcept;

	ControllerParam (int minimum, int maximum, int default_value, int zero_value, int denominator, const char* name,
					 float shown_min = 0.0f, float shown_max = 1.0f, int shown_decimals = 1, int step = 1) noexcept;

	ControllerParam (ControllerParam const& other) noexcept;

	ControllerParam&
	operator= (ControllerParam const& other);

	/**
	 * Return Adapter object. Never returns nullptr.
	 */
	Adapter*
	adapter() noexcept;

	Adapter const*
	adapter() const noexcept;

	int
	zero_value() const noexcept;

	int
	denominator() const noexcept;

	/**
	 * Return value divided by denominator and casted to float.
	 */
	float
	to_f() const noexcept;

	float
	shown_min() const noexcept;

	float
	shown_max() const noexcept;

	int
	shown_decimals() const noexcept;

	int
	step() const noexcept;

	/**
	 * Set from incoming value that is normalized to [0.0..1.0].
	 * Curve settings are applied by this function.
	 */
	void
	set_from_event (float value);

	/**
	 * Set value incoming from controller widget.
	 * Curve settings are applied by this function.
	 */
	void
	set_from_widget (int value);

	/**
	 * Adds sanitization of Adapter parameters.
	 */
	void
	sanitize();

	/*
	 * SaveableState reimplementation
	 */

	void
	save_state (QDomElement& parent) const;

	void
	load_state (QDomElement const& parent);

  private:
	Adapter	_adapter;
	int		_zero_value;
	int		_denominator;
	float	_1_div_denominator;
	float	_shown_min;
	float	_shown_max;
	int		_shown_decimals;
	int		_step;
};


inline int
ControllerParam::Adapter::forward (int in) const noexcept
{
	return renormalize (encurve (in), hard_limit_min, hard_limit_max, user_limit_min, user_limit_max);
}


inline int
ControllerParam::Adapter::reverse (int in) const noexcept
{
	return decurve (renormalize (in, user_limit_min, user_limit_max, hard_limit_min, hard_limit_max));
}


inline int
ControllerParam::Adapter::denormalize (float in) const noexcept
{
	return renormalize (in, 0.0f, 1.0f, 1.0f * hard_limit_min, 1.0f * hard_limit_max);
}


inline int
ControllerParam::Adapter::forward_normalized (float in) const noexcept
{
	return forward (denormalize (in));
}


inline
ControllerParam::ControllerParam (const char* name) noexcept:
	Param (name),
	_adapter (0, 0),
	_denominator (1),
	_1_div_denominator (1.0f / _denominator)
{ }


inline
ControllerParam::ControllerParam (int minimum, int maximum, int default_value, int zero_value, int denominator, const char* name,
								  float shown_min, float shown_max, int shown_decimals, int step) noexcept:
	Param (minimum, maximum, default_value, name),
	_adapter (minimum, maximum),
	_zero_value (zero_value),
	_denominator (denominator),
	_1_div_denominator (1.0f / _denominator),
	_shown_min (shown_min),
	_shown_max (shown_max),
	_shown_decimals (shown_decimals),
	_step (step)
{ }


inline
ControllerParam::ControllerParam (ControllerParam const& other) noexcept:
	Param (other),
	_adapter (other._adapter),
	_zero_value (other._zero_value),
	_denominator (other._denominator),
	_1_div_denominator (other._1_div_denominator),
	_shown_min (other._shown_min),
	_shown_max (other._shown_max),
	_shown_decimals (other._shown_decimals),
	_step (other._step)
{ }


inline ControllerParam&
ControllerParam::operator= (ControllerParam const& other)
{
	Param<int>::operator= (other);
	_adapter = other._adapter;
	_zero_value = other._zero_value;
	_denominator = other._denominator;
	_1_div_denominator = other._1_div_denominator;
	_shown_min = other._shown_min;
	_shown_max = other._shown_max;
	_shown_decimals = other._shown_decimals;
	_step = other._step;
	return *this;
}


inline ControllerParam::Adapter*
ControllerParam::adapter() noexcept
{
	return &_adapter;
}


inline ControllerParam::Adapter const*
ControllerParam::adapter() const noexcept
{
	return &_adapter;
}


inline int
ControllerParam::zero_value() const noexcept
{
	return _zero_value;
}


inline int
ControllerParam::denominator() const noexcept
{
	return _denominator;
}


inline float
ControllerParam::to_f() const noexcept
{
	return _1_div_denominator * get();
}


inline float
ControllerParam::shown_min() const noexcept
{
	return _shown_min;
}


inline float
ControllerParam::shown_max() const noexcept
{
	return _shown_max;
}


inline int
ControllerParam::shown_decimals() const noexcept
{
	return _shown_decimals;
}


inline int
ControllerParam::step() const noexcept
{
	return _step;
}


inline void
ControllerParam::set_from_event (float value)
{
	set (_adapter.forward (_adapter.denormalize (value)));
}


inline void
ControllerParam::set_from_widget (int value)
{
	set (_adapter.forward (value));
}

} // namespace Haruhi

#endif

