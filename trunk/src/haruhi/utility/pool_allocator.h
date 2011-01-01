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

#ifndef HARUHI__UTILITY__POOL_ALLOCATOR_H__INCLUDED
#define HARUHI__UTILITY__POOL_ALLOCATOR_H__INCLUDED

// Boost:
#include <boost/pool/pool_alloc.hpp>


/**
 * Use this macro inside class.
 */
#define USES_POOL_ALLOCATOR(T)											\
	private:															\
		typedef boost::pool_allocator<T> Allocator;						\
		static Allocator _allocator;									\
	public:																\
		void*															\
		operator new (std::size_t s)									\
			{ return _allocator.allocate (s); }							\
		void															\
		operator delete (void* p, std::size_t s)						\
			{ _allocator.deallocate (reinterpret_cast<T*> (p), s); }	\


/**
 * Use this macro in .cc file.
 */
#define POOL_ALLOCATOR_FOR(T) \
	T::Allocator T::_allocator;

#endif

