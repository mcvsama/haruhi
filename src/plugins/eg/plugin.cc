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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QLayout>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "plugin.h"


namespace EG {

Plugin::Plugin (std::string const& urn, std::string const& title, int id, QWidget* parent):
	Haruhi::Plugin (urn, title, id, parent)
{
}


Plugin::~Plugin()
{
}


void
Plugin::registered()
{
	enable();
}


void
Plugin::unregistered()
{
	panic();
}


void
Plugin::process()
{
	sync_inputs();
	clear_outputs();
}


void
Plugin::panic()
{
}


void
Plugin::graph_updated()
{
}


void
Plugin::set_unit_bay (Haruhi::UnitBay* unit_bay)
{
	UnitBayAware::set_unit_bay (unit_bay);
}


void
Plugin::save_state (QDomElement& element) const
{
}


void
Plugin::load_state (QDomElement const& element)
{
	disable();
	enable();
}

} // namespace EG

