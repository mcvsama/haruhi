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

#ifndef HARUHI__UNIT_H__INCLUDED
#define HARUHI__UNIT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>

// Qt:
#include <QtGui/QWidget>

// Haruhi:
#include <haruhi/core/unit.h>


namespace Haruhi {

class Session;
class UnitFactory;


class Unit: public Core::Unit, public QWidget
{
  public:
	Unit (UnitFactory*, Session*, std::string const& urn, std::string const& title, int id, QWidget* parent);

	/**
	 * Returns session object.
	 */
	Session*
	session() const { return _session; }

	void
	register_unit();

	void
	unregister_unit();

	UnitFactory*
	factory() const { return _factory; }

  private:
	UnitFactory*	_factory;
	Session*		_session;
};


class UnitFactory
{
  public:
	typedef std::map<std::string, std::string> InformationMap;

  public:
	UnitFactory();

	virtual ~UnitFactory() { }

	/**
	 * Allocates new unit and returns pointer to it.
	 * Allocated unit must be destroyed with destroy_unit().
	 */
	virtual Unit*
	create_unit (Session*, int id, QWidget* parent) = 0;

	/**
	 * Destroys given unit.
	 */
	virtual void
	destroy_unit (Unit*) = 0;

	/**
	 * Returns information about created units.
	 */
	virtual InformationMap const&
	information() const = 0;
};

} // namespace Haruhi

#endif

