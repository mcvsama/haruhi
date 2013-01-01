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

#ifndef HARUHI__COMPONENTS__PORTS_CONNECTOR__UNITS_COMBOBOX_H__INCLUDED
#define HARUHI__COMPONENTS__PORTS_CONNECTOR__UNITS_COMBOBOX_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QComboBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/graph.h>
#include <haruhi/graph/port.h>
#include <haruhi/graph/unit.h>
#include <haruhi/session/unit_bay.h>


namespace Haruhi {

class PortsConnector;

namespace PortsConnectorPrivate {

class UnitsCombobox: public QComboBox
{
	typedef std::vector<Unit*> Units;

  public:
	/**
	 * Creates combobox that observes unit_bay
	 * for inserting/removing units.
	 */
	UnitsCombobox (PortsConnector* ports_connector, Port::Direction type, QWidget* parent);

	~UnitsCombobox();

	/**
	 * Reads list of units registered in unit_bay
	 * and updates list.
	 */
	void
	read_units();

	/**
	 * Returns currently selected Unit*
	 * or 0 to indicate "no filtering".
	 */
	Unit*
	unit() const;

  private:
	PortsConnector*	_ports_connector;
	Units			_units;
	Port::Direction	_type;
};


inline Unit*
UnitsCombobox::unit() const
{
	return _units[currentItem()];
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

#endif

