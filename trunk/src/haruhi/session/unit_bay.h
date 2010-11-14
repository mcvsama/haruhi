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

#ifndef HARUHI__UNIT_BAY_H__INCLUDED
#define HARUHI__UNIT_BAY_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <set>

// Qt:
#include <QtGui/QWidget>

// Haruhi:
#include <haruhi/utility/signal.h>
#include <haruhi/graph/unit.h>


namespace Haruhi {

class UnitBay;
class Session;

/**
 * Mixin class.
 * Objects of this class can have assigned unit bay
 * and become UnitBay-aware.
 */
class UnitBayAware
{
  public:
	UnitBayAware();

	/**
	 * Assigns (or deassigns) UnitBay to object.
	 *
	 * \param	unit_bay
	 * 			UnitBay or 0.
	 */
	virtual void
	set_unit_bay (UnitBay* unit_bay);

	UnitBay*
	unit_bay() const { return _unit_bay; }

  private:
	UnitBay* _unit_bay;
};


/**
 * UnitBay is an Unit that contains other Units inside.
 */
class UnitBay:
	public Unit,
	public Signal::Receiver
{
  public:
	typedef std::set<Unit*> Units;

  public:
	UnitBay (Session*, std::string const& urn, std::string const& title, int id, QWidget* parent);

	virtual ~UnitBay();

	Units&
	units();

	Units const&
	units() const;

	void
	process();

	/*
	 * Unit API.
	 */

	void
	registered();

	void
	unregistered();

  private:
	void
	unit_unregistered (Unit*);

  private:
	Units _units;
};

} // namespace Haruhi

#endif

