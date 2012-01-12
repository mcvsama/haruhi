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
		typedef Atomic<unsigned int> This;

	  public:
		Atomic (unsigned int init = 0): _value (init)	{ }
		Atomic (This const& other)						{ store (other.load()); }
		This& operator= (This const& other)				{ store (other.load()); return *this; }

		unsigned int	load() const					{ return static_cast<unsigned int> (g_atomic_int_get (&_value)); }
		void			store (unsigned int value)		{ g_atomic_int_set (&_value, static_cast<int> (value)); }
		void			add (unsigned int value)		{ g_atomic_int_add (&_value, static_cast<int> (value)); }
		void			inc()							{ g_atomic_int_inc (&_value); }
		void			dec()							{ dec_and_test(); }

		/**
		 * Returns true if integer is 0 after decrementing it.
		 */
		bool			dec_and_test() const			{ return g_atomic_int_dec_and_test (&_value); }

	  private:
		gint mutable _value;
	};


template<>
	class Atomic<int>
	{
		typedef Atomic<int> This;

	  public:
		Atomic (int init = 0): _value (init)			{ }
		Atomic (This const& other)						{ store (other.load()); }
		This& operator= (This const& other)				{ store (other.load()); return *this; }

		int				load() const					{ return g_atomic_int_get (&_value); }
		void			store (int value)				{ g_atomic_int_set (&_value, value); }
		void			add (int value)					{ g_atomic_int_add (&_value, value); }
		void			inc()							{ g_atomic_int_inc (&_value); }
		void			dec()							{ dec_and_test(); }

		/**
		 * Returns true if integer is 0 after decrementing it.
		 */
		bool			dec_and_test() const			{ return g_atomic_int_dec_and_test (&_value); }

	  private:
		gint mutable _value;
	};


template<>
	class Atomic<bool>
	{
		typedef Atomic<bool> This;

	  public:
		Atomic (bool init = false): _value (init)		{ }
		Atomic (This const& other)						{ store (other.load()); }
		This& operator= (This const& other)				{ store (other.load()); return *this; }

		bool			load() const					{ return static_cast<bool> (_value.load()); }
		void			store (bool value)				{ _value.store (static_cast<int> (value)); }

	  private:
		Atomic<int> _value;
	};


template<>
	class Atomic<float>
	{
		typedef Atomic<float> This;

	  public:
		Atomic (float init = false): _value (init)		{ }
		Atomic (This const& other)						{ store (other.load()); }
		This& operator= (This const& other)				{ store (other.load()); return *this; }

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
		typedef Atomic<Type*> This;

	  public:
		Atomic (Type* init = 0): _value (init)			{ }
		Atomic (This const& other)						{ store (other.load()); }
		This& operator= (This const& other)				{ store (other.load()); return *this; }

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
			static_assert (sizeof (float) == sizeof (int), "bool has different size than int");
		}

		void
		operator= (float const& value)
		{
			union { float f; int i; } u;
			u.f = value;
			g_atomic_int_set (reinterpret_cast<int volatile*> (&_reference), u.i);
		}

		operator float() const
		{
			union { float f; int i; } u;
			u.i = g_atomic_int_get (reinterpret_cast<int volatile*> (&_reference));
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
			g_atomic_pointer_set (reinterpret_cast<gpointer volatile*> (&_reference), static_cast<gpointer> (value));
		}

		operator Type*() const
		{
			return static_cast<Type*> (g_atomic_pointer_get (reinterpret_cast<gpointer volatile*> (&_reference)));
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

