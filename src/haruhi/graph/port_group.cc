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

// Local:
#include "port_group.h"
#include "graph.h"


namespace Haruhi {

PortGroup::PortGroup (Graph* graph, std::string const& name):
	_graph (graph),
	_name (name)
{ }


std::string
PortGroup::name() const
{
	return _name;
}


void
PortGroup::set_name (std::string const& name)
{
	_name = name;
	_graph->port_group_renamed (this);
}


Graph*
PortGroup::graph() const
{
	return _graph;
}

} // namespace Haruhi

