/* vim:ts=4
 *
 * Copyleft 2010  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__CORE__PREDICATES_TCC__INCLUDED
#define HARUHI__CORE__PREDICATES_TCC__INCLUDED

namespace Haruhi {

namespace GraphDetail {

template<class Port>
	inline
	ConnectedTo<Port>::ConnectedTo (Port* port) noexcept:
		_port (port)
	{ }


template<class Port>
	inline bool
	ConnectedTo<Port>::operator() (argument_type const& port) const noexcept
	{
		return port->connected_to (_port);
	}


template<class Port>
	inline
	ConnectedFrom<Port>::ConnectedFrom (Port* port) noexcept:
		_port (port)
	{ }


template<class Port>
	inline bool
	ConnectedFrom<Port>::operator() (argument_type const& port) const noexcept
	{
		return _port->connected_to (port);
	}

} // namespace GraphDetail

} // namespace Haruhi

#endif

