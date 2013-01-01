/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>
#include <map>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/qdom.h>

// Local:
#include "conn_set.h"
#include "port.h"


namespace Haruhi {

ConnSet::ConnSet (bool only_internal):
	_only_internal (only_internal)
{
}


void
ConnSet::insert_unit (Unit* unit)
{
	_units.insert (unit);
}


void
ConnSet::remove_unit (Unit* unit)
{
	_units.erase (unit);
}


void
ConnSet::save()
{
	_connections.clear();
	for (Unit* u: _units)
	{
		for (Port const* f: u->outputs())
		{
			// Collect only forward connections:
			for (Port const* t: f->forward_connections())
			{
				if (_only_internal && _units.find (t->unit()) == _units.end())
					continue;
				_connections.push_back (Connection (QString ("%1").arg (f->unit()->id()), QString::fromStdString (f->full_name()),
													QString ("%1").arg (t->unit()->id()), QString::fromStdString (t->full_name())));
			}
		}
	}
}


void
ConnSet::load() const
{
	typedef std::map<QString, Unit*> Map;
	Map map;

	// Build map for fast access to Units:
	for (Unit* u: _units)
		map[QString ("%1").arg (u->id())] = u;

	Map::const_iterator m1, m2;
	for (Connection const& c: _connections)
	{
		m1 = map.find (c.source_unit);
		m2 = map.find (c.target_unit);
		if (m1 != map.end() && m2 != map.end())
		{
			// Find ports by full-name:
			Ports::const_iterator e1 = m1->second->outputs().end(), e2 = m2->second->inputs().end();
			Ports::const_iterator p1 = e1, p2 = e2;
			for (p1 = m1->second->outputs().begin(); p1 != e1; ++p1)
				if ((*p1)->full_name() == c.source_port.toStdString())
					break;
			if (p1 != e1)
			{
				for (p2 = m2->second->inputs().begin(); p2 != e2; ++p2)
					if ((*p2)->full_name() == c.target_port.toStdString())
						break;
				if (p2 != e2)
					(*p1)->connect_to (*p2);
			}
		}
	}
}


void
ConnSet::save_state (QDomElement& element) const
{
	for (Connection const& c: _connections)
	{
		QDomElement connection = element.ownerDocument().createElement ("connection");
		connection.setAttribute ("source-unit", c.source_unit);
		connection.setAttribute ("source-port", c.source_port);
		connection.setAttribute ("target-unit", c.target_unit);
		connection.setAttribute ("target-port", c.target_port);
		element.appendChild (connection);
	}
}


void
ConnSet::load_state (QDomElement const& element)
{
	_connections.clear();
	for (QDomElement& e: element)
	{
		if (e.tagName() == "connection")
		{
			_connections.push_back (Connection (e.attribute ("source-unit"), e.attribute ("source-port"),
												e.attribute ("target-unit"), e.attribute ("target-port")));
		}
	}
}

} // namespace Haruhi

