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

// Standard:
#include <cstddef>
#include <algorithm>
#include <string>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/exception.h>
#include <haruhi/utility/memory.h>

// Local:
#include "unit_loader.h"

// Factories:
#include <haruhi/units/mikuru/mikuru.h>
#include <haruhi/units/vanhalen/vanhalen.h>


namespace Haruhi {

UnitLoader::UnitLoader (Session* session):
	_session (session)
{
	_unit_factories.push_back (new MikuruFactory());
	_unit_factories.push_back (new VanHalenFactory());
}


UnitLoader::~UnitLoader()
{
	std::for_each (_unit_factories.begin(), _unit_factories.end(), delete_operator<UnitFactoryList::value_type>);
}


Unit*
UnitLoader::load (std::string const& urn, int id)
{
	UnitFactory* factory = find_factory (urn);
	if (factory)
	{
		Unit* unit = factory->create_unit (_session, id, 0);
		unit->hide();
		return unit;
	}
	return 0;
}


void
UnitLoader::unload (Unit* unit)
{
	UnitFactory* factory = find_factory (unit->urn());
	if (!factory)
		throw Exception ("couldn't find factory to unload unit");
	factory->destroy_unit (unit);
}


UnitLoader::UnitFactoryList const&
UnitLoader::unit_factories() const
{
	return _unit_factories;
}


UnitFactory*
UnitLoader::find_factory (std::string const& urn) const
{
	for (UnitFactoryList::const_iterator f = _unit_factories.begin();  f != _unit_factories.end();  ++f)
	{
		UnitFactory::InformationMap const& map = (*f)->information();
		UnitFactory::InformationMap::const_iterator i = map.find ("haruhi:urn");
		if (i == map.end())
			continue;
		if (i->second == urn)
			return *f;
	}
	return 0;
}

} // namespace Haruhi

