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

#ifndef HARUHI__UTILITY__PREDICATES_H__INCLUDED
#define HARUHI__UTILITY__PREDICATES_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdint.h>

// System:
#include <sys/time.h>


template<class tType>
	class PointerEquals
	{
	  public:
		typedef tType Type;

	  public:
		PointerEquals (Type* pointer):
			_pointer (pointer)
		{ }

		template<class Element>
			bool
			operator() (Element const& x)
			{
				return &x == _pointer;
			}

	  private:
		Type*	_pointer;
	};

#endif
