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
#include <haruhi/haruhi.h>
#include <haruhi/session.h>

// Local:
#include "unit.h"


namespace Haruhi {

Unit::Unit (UnitFactory* factory, Session* session, std::string const& urn, std::string const& title, int id, QWidget* parent):
	Core::Unit (session->graph(), urn, title, id),
	QWidget (parent),
	_factory (factory),
	_session (session)
{
	setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
}


void
Unit::register_unit()
{
	session()->graph()->register_unit (this);
}


void
Unit::unregister_unit()
{
	session()->graph()->unregister_unit (this);
}


UnitFactory::UnitFactory()
{ }

} // namespace Haruhi

