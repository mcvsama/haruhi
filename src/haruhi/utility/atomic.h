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
#include <haruhi/config/all.h>
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
		Atomic (unsigned int init = 0) noexcept: _value (init)	{ }
		Atomic (This const& other) noexcept						{ store (other.load()); }
		This& operator= (This const& other) noexcept			{ store (other.load()); return *this; }

		unsigned int	load() const noexcept					{ return static_cast<unsigned int> (g_atomic_int_get (&_value)); }
		void			store (unsigned int value) noexcept		{ g_atomic_int_set (&_value, static_cast<int> (value)); }
		void			add (unsigned int value) noexcept		{ g_atomic_int_add (&_value, static_cast<int> (value)); }
		void			inc() noexcept							{ g_atomic_int_inc (&_value); }
		void			dec() noexcept							{ dec_and_test(); }

		/**
		 * Returns true if integer is 0 after decrementing it.
		 */
		bool			dec_and_test() const noexcept			{ return g_atomic_int_dec_and_test (&_value); }

	  private:
		gint mutable _value;
	};


template<>
	class Atomic<int>
	{
		typedef Atomic<int> This;

	  public:
		Atomic (int init = 0) noexcept: _value (init)			{ }
		Atomic (This const& other) noexcept						{ store (other.load()); }
		This& operator= (This const& other) noexcept			{ store (other.load()); return *this; }

		int				load() const noexcept					{ return g_atomic_int_get (&_value); }
		void			store (int value) noexcept				{ g_atomic_int_set (&_value, value); }
		void			add (int value)	 noexcept				{ g_atomic_int_add (&_value, value); }
		void			inc() noexcept							{ g_atomic_int_inc (&_value); }
		void			dec() noexcept							{ dec_and_test(); }

		/**
		 * Returns true if integer is 0 after decrementing it.
		 */
		bool			dec_and_test() const noexcept			{ return g_atomic_int_dec_and_test (&_value); }

	  private:
		gint mutable _value;
	};


template<>
	class Atomic<bool>
	{
		typedef Atomic<bool> This;

	  public:
		Atomic (bool init = false) noexcept: _value (init)		{ }
		Atomic (This const& other) noexcept						{ store (other.load()); }
		This& operator= (This const& other) noexcept			{ store (other.load()); return *this; }

		bool			load() const noexcept					{ return static_cast<bool> (_value.load()); }
		void			store (bool value) noexcept				{ _value.store (static_cast<int> (value)); }

	  private:
		Atomic<int> _value;
	};


template<>
	class Atomic<float>
	{
		typedef Atomic<float> This;

	  public:
		Atomic (float init = false) noexcept: _value (init)		{ }
		Atomic (This const& other) noexcept						{ store (other.load()); }
		This& operator= (This const& other) noexcept			{ store (other.load()); return *this; }

		float
		load() const noexcept
		{
			union { float f; int i; } u;
			u.i = _value.load();
			return u.f;
		}

		void
		store (float value) noexcept
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
		Atomic (Type* init = 0) noexcept: _value (init)			{ }
		Atomic (This const& other) noexcept						{ store (other.load()); }
		This& operator= (This const& other) noexcept			{ store (other.load()); return *this; }

		Type*			load() const noexcept					{ return static_cast<Type*> (g_atomic_pointer_get (&_value)); }
		void			store (Type* value) noexcept			{ g_atomic_pointer_set (&_value, value); }

	  private:
		Type* _value;
	};

#endif

