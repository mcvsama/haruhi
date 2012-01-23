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

#ifndef HARUHI__GRAPH__PREDICATES_H__INCLUDED
#define HARUHI__GRAPH__PREDICATES_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "unit.h"


namespace Haruhi {

namespace GraphDetail {

template<class Port>
	class ConnectedTo
	{
	  public:
		typedef Port* argument_type;

	  public:
		explicit ConnectedTo (Port* port) noexcept;

		bool
		operator() (argument_type const& port) const noexcept;

	  private:
		Port* _port;
	};


template<class Port>
	inline ConnectedTo<Port>
	connected_to (Port* port) noexcept
	{
		return ConnectedTo<Port> (port);
	}


template<class Port>
	class ConnectedFrom
	{
	  public:
		typedef Port* argument_type;

	  public:
		explicit ConnectedFrom (Port* port) noexcept;

		bool
		operator() (argument_type const& port) const noexcept;

	  private:
		Port* _port;
	};


template<class Port>
	inline ConnectedFrom<Port>
	connected_from (Port* port) noexcept
	{
		return ConnectedFrom<Port> (port);
	}

} // namespace GraphDetail

} // namespace Haruhi

#include "predicates.tcc"

#endif

