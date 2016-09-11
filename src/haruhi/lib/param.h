/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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

namespace v06 {

/**
 * Defines common base methods for all types of parameters.
 */
class BasicParam: public SaveableState
{
  public:
	virtual ~BasicParam() = default;

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
	class Param: public BasicParam
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
		 * BasicParam implementation
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
		 * Emitted when parameter has changed:
		 */
		Signal::Emiter0 on_change;

		/**
		 * Emitted after on_change when parameter has changed.
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
		_value.store (other._value.load());
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

} // namespace v06


namespace v07 {

/**
 * Defines required methods for all types of parameters.
 */
class BasicParam: public SaveableState
{
  public:
	/**
	 * Ctor
	 *
	 * \param	identifier
	 * 			Unique identifier of the parameter. Used eg. in save-files.
	 */
	explicit
	BasicParam (std::string const& identifier);

	// Dtor
	virtual ~BasicParam() = default;

	/**
	 * Return parameter unique identifier.
	 */
	virtual std::string const&
	identifier() const noexcept;

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

  private:
	std::string	_identifier;
};


/**
 * Represent atomic parameter.
 *
 * \param	pValueType
 * 			Type of the internal parameter, usually int or bool.
 * 			Since it's used with atomic operations, it's necessary that it's a basic type that fit into CPU register.
 */
template<class pValueType>
	class Param: public BasicParam
	{
	  public:
		typedef pValueType	ValueType;

	  public:
		/**
		 * Ctor
		 *
		 * \param	identifier
		 *			BasicParam's identifier.
		 * \param	initial_value
		 *			Initial value for the parameter.
		 */
		explicit
		Param (std::string const& identifier, ValueType initial_value = ValueType());

		// Copy ctor
		Param (Param const& other) noexcept;

		/**
		 * Copy operator. Copies all information, including param's identifier, to be consistent with copy constructor.
		 */
		Param const&
		operator= (Param const& other) noexcept;

		/**
		 * Call set (new_value).
		 *
		 * \threadsafe
		 */
		void
		operator= (ValueType new_value) noexcept;

		/**
		 * Atomically get parameter's value.
		 *
		 * \threadsafe
		 */
		ValueType
		get() const noexcept;

		/**
		 * Atomically assign new value to the parameter.
		 *
		 * \threadsafe
		 */
		void
		set (ValueType new_value) noexcept;

		/*
		 * BasicParam implementation
		 */

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
		 * Emitted when parameter has changed:
		 */
		Signal::Emiter0 on_change;

		/**
		 * Emitted after on_change when parameter has changed.
		 * New value is given as parameter.
		 */
		Signal::Emiter1<ValueType> on_change_with_value;

	  private:
		Atomic<ValueType>	_value;
	};


inline
BasicParam::BasicParam (std::string const& identifier):
	_identifier (identifier)
{ }


inline std::string const&
BasicParam::identifier() const noexcept
{
	return _identifier;
}


template<class T>
	inline
	Param<T>::Param (std::string const& identifier, ValueType value):
		BasicParam (identifier),
		_value (value)
	{ }


template<class T>
	inline
	Param<T>::Param (Param const& other) noexcept:
		BasicParam (other.identifier()),
		_value (other.get())
	{ }


template<class T>
	inline Param<T> const&
	Param<T>::operator= (Param<T> const& other) noexcept
	{
		auto new_value = other.get();
		BasicParam::operator= (other);
		set (new_value);
		return *this;
	}


template<class T>
	inline void
	Param<T>::operator= (ValueType new_value) noexcept
	{
		set (new_value);
	}


template<class T>
	inline typename Param<T>::ValueType
	Param<T>::get() const noexcept
	{
		return _value.load (std::memory_order_relaxed);
	}


template<class T>
	inline void
	Param<T>::set (ValueType new_value) noexcept
	{
		_value.store (new_value, std::memory_order_relaxed);
		on_change();
		on_change_with_value (new_value);
	}


#if 0
template<class T>
	inline void
	Param<T>::reset() override;


template<class T>
	inline void
	Param<T>::sanitize() override;


template<class T>
	inline void
	Param<T>::save_state (QDomElement& parent) const override;


template<class T>
	inline void
	Param<T>::load_state (QDomElement const& parent) override;
#endif

} // namespace v07

} // namespace Haruhi

#endif

