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

#ifndef HARUHI__LIB__PARAM_H__INCLUDED
#define HARUHI__LIB__PARAM_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/signal.h>
#include <haruhi/utility/saveable_state.h>


namespace Haruhi {

/**
 * Defines common base methods for all types of parameters.
 */
class BaseParam: public SaveableState
{
  public:
	virtual ~BaseParam() { }

	/**
	 * Return parameter unique name.
	 */
	virtual const char*
	name() const = 0;

	/**
	 * Resets parameter to default value.
	 */
	virtual void
	reset() = 0;

	/**
	 * Enforces value to be between [minimum, maximum].
	 */
	virtual void
	sanitize() = 0;
};


/**
 * General type parameter supporting
 * min/max and sanitization.
 */
template<class tType>
	class Param: public BaseParam
	{
	  public:
		typedef tType Type;

	  public:
		Param (const char* name = "");

		Param (Type const& minimum, Type const& maximum, Type const& default_value, const char* name);

		Param (Param const& other);

		Param&
		operator= (Param const& other);

		const char*
		name() const;

		operator Type() const;

		void
		operator= (Type const& value);

		Type
		get() const;

		void
		set (Type const& value);

		Type
		minimum() const;

		Type
		maximum() const;

		Type
		default_value() const;

		/*
		 * BaseParam implementation
		 */

		void
		reset();

		void
		sanitize();

		/*
		 * SaveableState implementation
		 */

		void
		save_state (QDomElement& parent) const;

		void
		load_state (QDomElement const& parent);

	  public:
		/**
		 * Emited when parameter has changed:
		 */
		Signal::Emiter0 on_change;

		/**
		 * Emited after on_change when parameter has changed.
		 * New value is given as parameter.
		 */
		Signal::Emiter1<Type> on_change_with_value;

	  private:
		Type			_minimum;
		Type			_maximum;
		Type			_default_value;
		Atomic<Type>	_value;
		std::string		_name;
	};


template<class tType>
	Param<tType>::Param (const char* name):
		_minimum (0),
		_maximum (0),
		_default_value (0),
		_value (0),
		_name (name)
	{ }


template<class tType>
	inline
	Param<tType>::Param (Type const& minimum, Type const& maximum, Type const& default_value, const char* name):
		_minimum (minimum),
		_maximum (maximum),
		_default_value (default_value),
		_value (default_value),
		_name (name)
	{ }


template<class tType>
	Param<tType>::Param (Param const& other)
	{
		operator= (other);
	}


template<class tType>
	inline Param<tType>&
	Param<tType>::operator= (Param const& other)
	{
		_minimum = other._minimum;
		_maximum = other._maximum;
		_default_value = other._default_value;
		_value = other._value;
		_name = other._name;
		on_change();
		on_change_with_value (_value.load());
		return *this;
	}


template<class tType>
	inline const char*
	Param<tType>::name() const
	{
		return _name.c_str();
	}


template<class tType>
	inline
	Param<tType>::operator Type() const
	{
		return get();
	}


template<class tType>
	inline void
	Param<tType>::operator= (Type const& value)
	{
		set (value);
	}


template<class tType>
	inline tType
	Param<tType>::get() const
	{
		return _value.load();
	}


template<class tType>
	inline void
	Param<tType>::set (Type const& value)
	{
		_value.store (value);
		on_change();
		on_change_with_value (value);
	}


template<class tType>
	inline tType
	Param<tType>::minimum() const
	{
		return _minimum;
	}


template<class tType>
	inline tType
	Param<tType>::maximum() const
	{
		return _maximum;
	}


template<class tType>
	inline tType
	Param<tType>::default_value() const
	{
		return _default_value;
	}


template<class tType>
	inline void
	Param<tType>::reset()
	{
		set (_default_value);
	}


template<class tType>
	inline void
	Param<tType>::sanitize()
	{
		set (bound (get(), _minimum, _maximum));
	}


template<class tType>
	inline void
	Param<tType>::save_state (QDomElement& parent) const
	{
		parent.setAttribute ("value", QString ("%1").arg (get()));
		parent.appendChild (parent.ownerDocument().createTextNode (QString::number (get())));
	}


template<class tType>
	inline void
	Param<tType>::load_state (QDomElement const& parent)
	{
		set (bound<int> (parent.text().toInt(), minimum(), maximum()));
		sanitize();
	}

} // namespace Haruhi

#endif

