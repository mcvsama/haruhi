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

#ifndef HARUHI__UTILITY__ATOMIC_H__INCLUDED
#define HARUHI__UTILITY__ATOMIC_H__INCLUDED

// Standard:
#include <cstddef>

// System:
#include <glib.h>


template<class Type>
	class Atomic;


template<>
	class Atomic<unsigned int>
	{
	  public:
		Atomic (unsigned int volatile& reference):
			_reference (reference)
		{ }

		void
		operator= (unsigned int const& value)
		{
			g_atomic_int_set (&_reference, static_cast<int> (value));
		}

		operator unsigned int() const
		{
			return static_cast<unsigned int> (g_atomic_int_get (&_reference));
		}

	  private:
		unsigned int volatile& _reference;
	};


template<>
	class Atomic<int>
	{
	  public:
		Atomic (int volatile& reference):
			_reference (reference)
		{ }

		void
		operator= (int const& value)
		{
			g_atomic_int_set (&_reference, value);
		}

		operator int() const
		{
			return g_atomic_int_get (&_reference);
		}

	  private:
		int volatile& _reference;
	};


/**
 * Note: will it blend?
 */
template<class Type>
	class Atomic<Type*>
	{
	  public:
		Atomic (Type* volatile& reference):
			_reference (reference)
		{ }

		void
		operator= (Type* const& value)
		{
			g_atomic_pointer_set (reinterpret_cast<volatile gpointer*> (&_reference), static_cast<gpointer> (value));
		}

		operator Type*() const
		{
			return static_cast<Type*> (g_atomic_pointer_get (reinterpret_cast<volatile gpointer*> (&_reference)));
		}

	  private:
		Type* volatile& _reference;
	};


template<class Type>
	class Atomic
	{
	  public:
		Atomic (Type volatile& reference):
			_reference (reference)
		{ }

		void
		operator= (Type const& value)
		{
			_reference = value;
		}

		operator Type() const
		{
			return _reference;
		}

	  private:
		Type volatile& _reference;
	};


template<class Type>
	inline Atomic<Type>
	atomic (Type volatile& reference)
	{
		return Atomic<Type> (reference);
	}

#endif

