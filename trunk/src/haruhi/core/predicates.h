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

#ifndef HARUHI__CORE__PREDICATES_H__INCLUDED
#define HARUHI__CORE__PREDICATES_H__INCLUDED

// Standard:
#include <cstddef>

// Local:
#include "unit.h"


namespace Core {

template<class Port>
	class ConnectedTo
	{
	  public:
		typedef Port* argument_type;

	  public:
		explicit ConnectedTo (Port* port);

		bool
		operator() (argument_type const& port) const;

	  private:
		Port* _port;
	};


template<class Port>
	ConnectedTo<Port>
	connected_to (Port* port)
	{
		return ConnectedTo<Port> (port);
	}


template<class Port>
	class ConnectedFrom
	{
	  public:
		typedef Port* argument_type;

	  public:
		explicit ConnectedFrom (Port* port);

		bool
		operator() (argument_type const& port) const;

	  private:
		Port* _port;
	};


template<class Port>
	ConnectedFrom<Port>
	connected_from (Port* port)
	{
		return ConnectedFrom<Port> (port);
	}

} // namespace Core

#include "predicates.tcc"

#endif

