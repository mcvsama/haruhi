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

// Haruhi:
#include <haruhi/config/system.h>


/*
 * Atomic types
 */

template<class Type>
	class Atomic;


template<>
	class Atomic<unsigned int>
	{
	  public:
		Atomic (unsigned int init = 0): _value (init)	{ }
		unsigned int	load() const					{ return static_cast<unsigned int> (g_atomic_int_get (&_value)); }
		void			store (unsigned int value)		{ g_atomic_int_set (&_value, static_cast<int> (value)); }

	  private:
		unsigned int mutable _value;
	};


template<>
	class Atomic<int>
	{
	  public:
		Atomic (int init = 0): _value (init)			{ }
		int				load() const					{ return g_atomic_int_get (&_value); }
		void			store (int value)				{ g_atomic_int_set (&_value, value); }

	  private:
		int mutable _value;
	};


template<>
	class Atomic<bool>
	{
	  public:
		Atomic (bool init = false): _value (init)		{ }
		bool			load() const					{ return static_cast<bool> (_value.load()); }
		void			store (bool value)				{ _value.store (static_cast<int> (value)); }

	  private:
		Atomic<int> _value;
	};


template<>
	class Atomic<float>
	{
	  public:
		Atomic (float init = false): _value (init)
		{ }

		float
		load() const
		{
			union { float f; int i; } u;
			u.i = _value.load();
			return u.f;
		}

		void
		store (float value)
		{
			union { float f; int i; } u;
			u.f = value;
			_value.store (u.i);
		}

	  private:
		Atomic<int> _value;
	};


template<class Type>
	class Atomic<Type*>
	{
	  public:
		Atomic (Type* init = 0): _value (init)			{ }
		Type*			load() const					{ return static_cast<Type*> (g_atomic_pointer_get (&_value)); }
		void			store (Type* value)				{ g_atomic_pointer_set (&_value, value); }

	  private:
		Type* _value;
	};


/*
 * Atomic operations
 */

template<class Type>
	class AtomicOperation;


template<>
	class AtomicOperation<unsigned int>
	{
	  public:
		AtomicOperation (unsigned int volatile& reference):
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
	class AtomicOperation<int>
	{
	  public:
		AtomicOperation (int volatile& reference):
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


template<>
	class AtomicOperation<float>
	{
	  public:
		AtomicOperation (float volatile& reference):
			_reference (reference)
		{
			// To support AtomicOperation<float> sizeof float must be equal to sizeof int.
			static_assert (sizeof (float) == sizeof (int), bool_has_different_size_than_int);
		}

		void
		operator= (float const& value)
		{
			union { float f; int i; } u;
			u.f = value;
			g_atomic_int_set (&_reference, u.i);
		}

		operator float() const
		{
			union { float f; int i; } u;
			u.i = g_atomic_int_get (&_reference);
			return u.f;
		}

	  private:
		float volatile& _reference;
	};


/**
 * Note: will it blend?
 */
template<class Type>
	class AtomicOperation<Type*>
	{
	  public:
		AtomicOperation (Type* volatile& reference):
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
	inline AtomicOperation<Type>
	atomic (Type volatile& reference)
	{
		return AtomicOperation<Type> (reference);
	}

#endif

