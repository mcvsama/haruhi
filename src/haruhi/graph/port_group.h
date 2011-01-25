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

#ifndef HARUHI__GRAPH__PORT_GROUP_H__INCLUDED
#define HARUHI__GRAPH__PORT_GROUP_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>


namespace Haruhi {

class Graph;

class PortGroup
{
  public:
	struct CompareByName
	{
		bool
		operator() (PortGroup* a, PortGroup* b)
		{
			return a->name() < b->name();
		}
	};

  public:
	PortGroup (Graph* graph, std::string const& name);

	std::string
	name() const;

	void
	set_name (std::string const&);

	Graph*
	graph() const;

  private:
	Graph*		_graph;
	std::string	_name;
};

} // namespace Haruhi

#endif

