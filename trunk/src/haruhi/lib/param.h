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

#ifndef HARUHI__LIB__PARAM_H__INCLUDED
#define HARUHI__LIB__PARAM_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/numeric.h>


namespace Haruhi {

/**
 * General type parameter supporting
 * min/max and sanitization.
 */
template<class tType>
	class Param
	{
	  public:
		typedef tType Type;

	  public:
		Param():
			_minimum (0),
			_maximum (0),
			_default_value (0),
			_value (0)
		{ }

		Param (Type const& minimum, Type const& maximum, Type const& default_value):
			_minimum (minimum),
			_maximum (maximum),
			_default_value (default_value),
			_value (default_value)
		{ }

		Param (Param const& other) { operator= (other); }

		Param&
		operator= (Param const& other)
		{
			_minimum = other._minimum;
			_maximum = other._maximum;
			_default_value = other._default_value;
			_value = other._value;
			return *this;
		}

		operator Type() const { return get(); }

		void
		operator= (Type const& value) { set (value); }

		Type
		get() const { return _value.load(); }

		void
		set (Type const& value) { _value.store (value); }

		Type
		minimum() const { return _minimum; }

		Type
		maximum() const { return _maximum; }

		Type
		default_value() const { return _default_value; }

		/**
		 * Resets parameter to default value.
		 */
		virtual void
		reset() { set (_default_value); }

		/**
		 * Enforces value to be between [minimum, maximum].
		 */
		virtual void
		sanitize() { set (bound (get(), _minimum, _maximum)); }

		/**
		 * Returns pointer to value parameter.
		 */
		volatile int*
		parameter() { return &_value; }

	  private:
		Type			_minimum;
		Type			_maximum;
		Type			_default_value;
		Atomic<Type>	_value;
	};

} // namespace Haruhi

#endif

