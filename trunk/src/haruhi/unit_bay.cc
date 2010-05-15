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

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/session.h>

// Local:
#include "unit_bay.h"


namespace Haruhi {

UnitBayAware::UnitBayAware():
	_unit_bay (0)
{
}


void
UnitBayAware::set_unit_bay (UnitBay* unit_bay)
{
	_unit_bay = unit_bay;
}


UnitBay::UnitBay (Session* session, std::string const& urn, std::string const& title, int id, QWidget* parent):
	Unit (0, session, urn, title, id, parent)
{
	register_unit();
	graph()->unit_unregistered.connect (this, &UnitBay::unit_unregistered);
}


UnitBay::~UnitBay()
{
	unregister_unit();
}


void
UnitBay::process()
{
}


UnitBay::Units&
UnitBay::units()
{
	return _units;
}


UnitBay::Units const&
UnitBay::units() const
{
	return _units;
}


void
UnitBay::unit_unregistered (Core::Unit* core_unit)
{
	Unit* unit = dynamic_cast<Unit*> (core_unit);
	// Remove from UnitBay, if it is owned sync:
	if (unit)
		_units.erase (unit);
}

} // namespace Haruhi

