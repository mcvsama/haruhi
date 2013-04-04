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
 * -
 * There are two Shared classes. First is general purpose, slower implementation
 * that uses additional proxy object (Data) for reference counting.
 * Second is specialized version for objects that inherit class FastShared
 * (that itself provides reference counting).
 *
 * This implementation uses trick described in following article:
 *  http://stackoverflow.com/questions/281725/template-specialization-based-on-inherit-class
 * It allows specialization of Shared for objects that inherit certain class
 * (FastShared in this case).
 */

#ifndef HARUHI__UTILITY__SHARED_H__INCLUDED
#define HARUHI__UTILITY__SHARED_H__INCLUDED

// Standard:
#include <cstddef>
#include <type_traits>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/atomic.h>


/**
 * Implements general purpose shared pointer proxy (reference counting proxy),
 * automatically deletes owned pointer when no object uses it.
 *
 * No two Shared object can be created from the same C-pointer,
 * otherwise behavior will be undefined. The only valid way to create
 * second Shared object is to copy existing Shared (by use of assignment
 * operator or copy constructor).
 *
 * This class is not completely thread safe, although
 * reference counting is handled in thread safe manner (using Atomic types).
 *
 * So it is safe to simultaneously read (accessed using const methods/operators)
 * the same Shared from multiple threads.
 *
 * It is safe to write (access using mutable methods/operators) to _different_
 * Shareds in multiple threads simultaneously, even if these Shareds are copies
 * (hold reference to the same data).
 */
template<class T, class = void>
	class Shared
	{
	  public:
		typedef T Type;

	  private:
		struct Data
		{
			Type*		object;
			Atomic<int>	references;

			Data (Type* object) noexcept:
				object (object),
				references (1)
			{ }

			~Data() noexcept
			{
				delete object;
			}
		};

	  public:
		Shared (Type* object = 0):
			_data (new Data (object))
		{ }

		Shared (Shared const& other) noexcept
		{
			acquire_data (other._data);
		}

		~Shared() noexcept
		{
			leave_data();
		}

		Shared&
		operator= (Shared const& other) noexcept
		{
			leave_data();
			acquire_data (other._data);
			return *this;
		}

		/**
		 * As having distinct Shared objects (not created by copying)
		 * is forbidden, we say that two Shareds are the same
		 * when their C-pointers are the same.
		 */
		bool
		operator== (Shared const& other) const noexcept
		{
			return _data == other._data;
		}

		bool
		operator< (Shared const& other) const noexcept
		{
			return _data < other._data;
		}

		Type*
		operator->() const noexcept { return _data->object; }

		Type*
		get() const noexcept { return _data->object; }

		Type&
		operator*() noexcept { return *_data->object; }

		Type const&
		operator*() const noexcept { return *_data->object; }

		bool
		operator!() const noexcept { return !_data->object; }

	  private:
		void
		leave_data() noexcept
		{
			if (--_data->references == 0)
				delete _data;
		}

		void
		acquire_data (Data* data) noexcept
		{
			_data = data;
			++_data->references;
		}

	  private:
		Data* _data;
	};


/**
 * Inherit this class if you want to use faster Shared.
 * Specialized version of Shared<> will be used for your class.
 * Reference counting is done in this object itself (in FastShared),
 * so it's not necessary to hold proxy object for reference counting.
 */
class FastShared
{
	template<class, class>
		friend class Shared;

	Atomic<int>	references;

  public:
	FastShared() noexcept:
		references (1)
	{ }
};


/**
 * Specialized version of Shared<> for objects that inherit
 * FastShared. Faster than standard Shared as it does not
 * use additional proxy object for reference counting.
 *
 * The same notes apply for this class about thread safety
 * as for standard (non-specialized) version of Shared.
 */
template<class T>
	class Shared<T, class std::enable_if<std::is_base_of<FastShared, T>::value>::type>
	{
	  public:
		typedef T Type;

	  public:
		Shared (Type* object) noexcept:
			_object (object)
		{ }

		Shared (Shared const& other) noexcept
		{
			acquire_data (other._object);
		}

		~Shared() noexcept
		{
			leave_data();
		}

		Shared&
		operator= (Shared const& other) noexcept
		{
			leave_data();
			acquire_data (other._object);
			return *this;
		}

		/**
		 * As having distinct Shared objects (not created by copying)
		 * is forbidden, we say that two Shareds are the same
		 * when their C-pointers are the same.
		 */
		bool
		operator== (Shared const& other) const noexcept
		{
			return _object == other._object;
		}

		bool
		operator< (Shared const& other) const noexcept
		{
			return _object < other._object;
		}

		Type*
		operator->() const noexcept { return _object; }

		Type*
		get() const noexcept { return _object; }

		Type&
		operator*() noexcept { return *_object; }

		Type const&
		operator*() const noexcept { return *_object; }

		bool
		operator!() const noexcept { return !_object; }

	  private:
		void
		leave_data() noexcept
		{
			if (--_object->FastShared::references == 0)
				delete _object;
		}

		void
		acquire_data (Type* object) noexcept
		{
			_object = object;
			++_object->FastShared::references;
		}

	  private:
		Type* _object;
	};


/**
 * Capture given pointer and return Shared pointer.
 * Takes ownership of the parameter.
 */
template<class Type>
	inline Shared<Type>
	shared (Type const& pointer) noexcept
	{
		return Shared<Type> (pointer);
	}

#endif

