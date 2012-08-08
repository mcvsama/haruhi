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

#ifndef HARUHI__LIB__PARAM_H__INCLUDED
#define HARUHI__LIB__PARAM_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/signal.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/range.h>


namespace Haruhi {

/**
 * Defines common base methods for all types of parameters.
 */
class BaseParam: public SaveableState
{
  public:
	virtual ~BaseParam() = default;

	/**
	 * Return parameter unique name.
	 */
	virtual const char*
	name() const noexcept = 0;

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
		constexpr
		Param (const char* name = "") noexcept;

		constexpr
		Param (Range<Type> range, Type default_value, const char* name) noexcept;

		Param (Param const& other) noexcept;

		Param&
		operator= (Param const& other) noexcept;

		operator Type() const noexcept;

		void
		operator= (Type const& value);

		Type
		get() const noexcept;

		void
		set (Type const& value);

		Type
		minimum() const noexcept;

		Type
		maximum() const noexcept;

		Range<Type> const&
		range() const noexcept;

		Type
		default_value() const noexcept;

		/*
		 * BaseParam implementation
		 */

		const char*
		name() const noexcept override;

		void
		reset() override;

		void
		sanitize() override;

		/*
		 * SaveableState implementation
		 */

		void
		save_state (QDomElement& parent) const override;

		void
		load_state (QDomElement const& parent) override;

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
		Range<Type>		_range;
		Type			_default_value;
		Atomic<Type>	_value;
		std::string		_name;
	};


template<class tType>
	inline constexpr
	Param<tType>::Param (const char* name) noexcept:
		_range (0, 0),
		_default_value (0),
		_value (0),
		_name (name)
	{ }


template<class tType>
	inline constexpr
	Param<tType>::Param (Range<Type> range, Type default_value, const char* name) noexcept:
		_range (range),
		_default_value (default_value),
		_value (default_value),
		_name (name)
	{ }


template<class tType>
	inline
	Param<tType>::Param (Param const& other) noexcept
	{
		operator= (other);
	}


template<class tType>
	inline Param<tType>&
	Param<tType>::operator= (Param const& other) noexcept
	{
		_range = other._range;
		_default_value = other._default_value;
		_value = other._value;
		_name = other._name;
		on_change();
		on_change_with_value (_value.load());
		return *this;
	}


template<class tType>
	inline
	Param<tType>::operator Type() const noexcept
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
	Param<tType>::get() const noexcept
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
	Param<tType>::minimum() const noexcept
	{
		return _range.min();
	}


template<class tType>
	inline tType
	Param<tType>::maximum() const noexcept
	{
		return _range.max();
	}


template<class tType>
	inline Range<tType> const&
	Param<tType>::range() const noexcept
	{
		return _range;
	}


template<class tType>
	inline tType
	Param<tType>::default_value() const noexcept
	{
		return _default_value;
	}


template<class tType>
	inline const char*
	Param<tType>::name() const noexcept
	{
		return _name.c_str();
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
		set (bound (get(), _range));
	}


template<class tType>
	inline void
	Param<tType>::save_state (QDomElement& parent) const
	{
		parent.appendChild (parent.ownerDocument().createTextNode (QString::number (get())));
	}


template<class tType>
	inline void
	Param<tType>::load_state (QDomElement const& parent)
	{
		set (bound<int> (parent.text().toInt(), _range));
		sanitize();
	}

} // namespace Haruhi

#endif

