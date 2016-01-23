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
#include <algorithm>

// Qt:
#include <QtGui/QComboBox>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "units_combobox.h"
#include "ports_connector.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

UnitsCombobox::UnitsCombobox (PortsConnector* ports_connector, Port::Direction type, QWidget* parent):
	QComboBox (parent),
	_ports_connector (ports_connector),
	_type (type)
{
	read_units();
	setIconSize (Resources::Icons16::haruhi().size());
}


UnitsCombobox::~UnitsCombobox()
{
}


void
UnitsCombobox::read_units()
{
	QPixmap unit_icon = (_type == Port::Input) ? Resources::Icons16::input_unit() : Resources::Icons16::output_unit();

	clear();
	_units.clear();

	_units.push_back (0); // 0 is for "All units".
	addItem (Resources::Icons16::all(), "All units", 0);

	Units list;

	// PortsConnector's external units:
	list = Units (_ports_connector->_external_units.begin(), _ports_connector->_external_units.end());
	std::sort (list.begin(), list.end(), Unit::compare_by_title);
	for (Unit* u: list)
	{
		if (((_type == Port::Input) ? u->inputs() : u->outputs()).empty())
			continue;
		addItem (unit_icon, QString::fromStdString (u->title()));
		_units.push_back (u);
	}

	// UnitBay's units:
	list = Units (_ports_connector->_unit_bay->units().begin(), _ports_connector->_unit_bay->units().end());
	std::sort (list.begin(), list.end(), Unit::compare_by_title);
	for (Unit* u: list)
	{
		if (((_type == Port::Input) ? u->inputs() : u->outputs()).empty())
			continue;
		addItem (unit_icon, QString::fromStdString (u->title()), static_cast<qulonglong> (_units.size()));
		_units.push_back (u);
	}
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

