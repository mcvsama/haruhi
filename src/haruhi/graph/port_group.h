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

#ifndef HARUHI__GRAPH__PORT_GROUP_H__INCLUDED
#define HARUHI__GRAPH__PORT_GROUP_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

class Graph;

class PortGroup
{
  public:
	PortGroup (Graph* graph, std::string const& name) noexcept;

	/**
	 * Return port group name.
	 */
	std::string
	name() const;

	/**
	 * Rename port group.
	 * Needs Graph lock.
	 */
	void
	set_name (std::string const&);

	Graph*
	graph() const noexcept;

	/**
	 * Helper for ordering.
	 */
	static bool
	compare_by_name (PortGroup const* first, PortGroup const* second) noexcept;

  private:
	Graph*		_graph;
	std::string	_name;
};


inline std::string
PortGroup::name() const
{
	return _name;
}


inline Graph*
PortGroup::graph() const noexcept
{
	return _graph;
}


inline bool
PortGroup::compare_by_name (PortGroup const* first, PortGroup const* second) noexcept
{
	return first->name() < second->name();
}

} // namespace Haruhi

#endif

