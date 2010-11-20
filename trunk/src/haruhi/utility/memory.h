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

#ifndef HARUHI__UTILITY__MEMORY_H__INCLUDED
#define HARUHI__UTILITY__MEMORY_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/utility/mutex.h>


template<class Type>
	void delete_operator (Type element)
	{
		delete element;
	}


/**
 * Implements shared pointer proxy (reference counting proxy),
 * automatically deleted owned pointer when no object uses it.
 *
 * No two Shared object can be created from the same C-pointer,
 * otherwise behavior will be undefined. The only valid way to create
 * second Shared object is to copy existing Shared (by use of assignment
 * operator or copy constructor).
 *
 * This class is not completely thread safe, although
 * reference counting is handled in thread safe manner (using Mutex).
 *
 * So it is safe to simultaneously read (accessed using const methods/operators)
 * the same Shared from multiple threads.
 *
 * It is safe to write (access using mutable methods/operators) to _different_
 * Shareds in multiple threads simultaneously, even if these Shareds are copies
 * (hold reference to the same data).
 */
template<class T>
	class Shared
	{
	  public:
		typedef T Type;

	  private:
		struct Data: public Mutex
		{
			Type*	object;
			int		references;

			Data (Type* object):
				object (object),
				references (1)
			{ }

			~Data()
			{
				if (empty())
					delete object;
			}

			void
			increase()
			{
				lock();
				++references;
				unlock();
			}

			void
			decrease()
			{
				lock();
				--references;
				unlock();
			}

			bool
			empty() const
			{
				lock();
				int r = references;
				unlock();
				return r == 0;
			}
		};

	  public:
		Shared (Type* object = 0):
			_data (new Data (object))
		{ }

		Shared (Shared const& other)
		{
			acquire_data (other._data);
		}

		~Shared()
		{
			leave_data();
		}

		Shared&
		operator= (Shared const& other)
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
		operator== (Shared const& other) const
		{
			return _data == other._data;
		}

		bool
		operator< (Shared const& other) const
		{
			return _data == other._data;
		}

		Type*
		operator->() const { return _data->object; }

		Type*
		get() const { return _data->object; }

		Type&
		operator*() { return *_data->object; }

		Type const&
		operator*() const { return *_data->object; }

		bool
		operator!() const { return !_data->object; }

	  private:
		void
		leave_data()
		{
			_data->decrease();
			if (_data->empty())
				delete _data;
		}

		void
		acquire_data (Data* data)
		{
			_data = data;
			_data->increase();
		}

	  private:
		Data* _data;
	};


template<class Type>
	inline Shared<Type>
	shared (Type const& pointer)
	{
		return Shared<Type> (pointer);
	}

#endif

