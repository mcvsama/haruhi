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

// Qt:
#include <QtGui/QLayout>
#include <QtGui/QComboBox>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/session.h>

// Local:
#include "units_combobox.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

UnitsCombobox::UnitsCombobox (UnitBay* unit_bay, Core::Port::Direction type, QWidget* parent):
	QComboBox (parent),
	_unit_bay (unit_bay),
	_type (type)
{
	read_units();
	setFixedHeight (27); // XXX fix for ugly Qt style (QtCurve)
}


UnitsCombobox::~UnitsCombobox()
{
}


void
UnitsCombobox::read_units()
{
	QPixmap unit_icon = (_type == Core::Port::Input) ? Config::Icons16::input_unit() : Config::Icons16::output_unit();

	clear();
	_units.clear();

	_units.push_back (0); // 0 is for "All units".
	if (_unit_bay->session()->audio_backend())
		_units.push_back (_unit_bay->session()->audio_backend());
	if (_unit_bay->session()->event_backend())
		_units.push_back (_unit_bay->session()->event_backend());

	insertItem (Config::Icons16::all(), "All units", 0);
	insertItem (unit_icon, QString::fromStdString (_units[1]->title()));
	insertItem (unit_icon, QString::fromStdString (_units[2]->title()));

	// UnitBay's units:
	Units list (_unit_bay->units().begin(), _unit_bay->units().end());
	std::sort (list.begin(), list.end(), Core::Unit::ComparePointerByTitle());
	for (Units::iterator u = list.begin(); u != list.end(); ++u)
	{
		insertItem (unit_icon, QString::fromStdString ((*u)->title()), _units.size());
		_units.push_back (*u);
	}
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

