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

#ifndef HARUHI__UNIT_LOADER_H__INCLUDED
#define HARUHI__UNIT_LOADER_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/unit.h>


namespace Haruhi {

class Session;


/**
 * Manages units factories.
 */
class UnitLoader
{
  public:
	typedef std::list<UnitFactory*> UnitFactoryList;

  public:
	UnitLoader (Session*);

	~UnitLoader();

	Unit*
	load (std::string const& urn, int id = 0);

	void
	unload (Unit*);

	UnitFactoryList const&
	unit_factories() const;

  private:
	UnitFactory*
	find_factory (std::string const& urn) const;

  private:
	Session*		_session;
	UnitFactoryList	_unit_factories;
};

} // namespace Haruhi

#endif

