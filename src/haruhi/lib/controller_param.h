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
#include <haruhi/utility/range.h>
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
		Adapter (Range<int> limit, int center_value) noexcept;

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
		float		curve;
		Range<int>	hard_limit;	// Hard limit is hardcoded limit for parameter.
		Range<int>	user_limit;	// User limit is user-defined convenience limit.

	  private:
		int			_center;
	};

  public:
	explicit
	ControllerParam (const char* name = "") noexcept;

	ControllerParam (Range<int> range, int default_value, int center_value, int denominator, const char* name,
					 Range<float> shown_range = Range<float> { 0.0f, 1.0f }, int shown_decimals = 1, int step = 1) noexcept;

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
	center_value() const noexcept;

	int
	denominator() const noexcept;

	/**
	 * Return value divided by denominator and casted to float.
	 */
	float
	to_f() const noexcept;

	Range<float>
	shown_range() const noexcept;

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
	sanitize() override;

	/*
	 * SaveableState reimplementation
	 */

	void
	save_state (QDomElement& parent) const override;

	void
	load_state (QDomElement const& parent) override;

  private:
	Adapter			_adapter;
	int				_center_value			= 0;
	int				_denominator			= 1;
	float			_1_div_denominator		= 1.0f;
	Range<float>	_shown_range			= { 0.0f, 1.0f };
	int				_shown_decimals			= 0;
	int				_step					= 1;
};


inline int
ControllerParam::Adapter::forward (int in) const noexcept
{
	return renormalize (encurve (in), hard_limit, user_limit);
}


inline int
ControllerParam::Adapter::reverse (int in) const noexcept
{
	return decurve (renormalize (in, user_limit, hard_limit));
}


inline int
ControllerParam::Adapter::denormalize (float in) const noexcept
{
	return renormalize (in, { 0.0f, 1.0f }, hard_limit);
}


inline int
ControllerParam::Adapter::forward_normalized (float in) const noexcept
{
	return forward (denormalize (in));
}


inline
ControllerParam::ControllerParam (const char* name) noexcept:
	Param (name),
	_adapter ({ 0, 0 }, 0),
	_denominator (1),
	_1_div_denominator (1.0f / _denominator)
{ }


inline
ControllerParam::ControllerParam (Range<int> range, int default_value, int center_value, int denominator, const char* name,
								  Range<float> shown_range, int shown_decimals, int step) noexcept:
	Param (range, default_value, name),
	_adapter (range, center_value),
	_center_value (center_value),
	_denominator (denominator),
	_1_div_denominator (1.0f / _denominator),
	_shown_range (shown_range),
	_shown_decimals (shown_decimals),
	_step (step)
{ }


inline
ControllerParam::ControllerParam (ControllerParam const& other) noexcept:
	Param (other),
	_adapter (other._adapter),
	_center_value (other._center_value),
	_denominator (other._denominator),
	_1_div_denominator (other._1_div_denominator),
	_shown_range (other._shown_range),
	_shown_decimals (other._shown_decimals),
	_step (other._step)
{ }


inline ControllerParam&
ControllerParam::operator= (ControllerParam const& other)
{
	Param<int>::operator= (other);
	_adapter = other._adapter;
	_center_value = other._center_value;
	_denominator = other._denominator;
	_1_div_denominator = other._1_div_denominator;
	_shown_range = other._shown_range;
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
ControllerParam::center_value() const noexcept
{
	return _center_value;
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


inline Range<float>
ControllerParam::shown_range() const noexcept
{
	return _shown_range;
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

