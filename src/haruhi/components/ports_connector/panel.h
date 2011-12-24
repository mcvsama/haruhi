/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__COMPONENTS__PORTS_CONNECTOR__PANEL_H__INCLUDED
#define HARUHI__COMPONENTS__PORTS_CONNECTOR__PANEL_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>

// Local:
#include "ports_connector.h"
#include "units_combobox.h"


namespace Haruhi {

class PortsConnector;

namespace PortsConnectorPrivate {

class PortsList;

class Panel: public QWidget
{
	Q_OBJECT

  public:
	Panel (Port::Direction type, PortsConnector* ports_connector, QWidget* parent);

	PortsList*
	list() const;

	UnitsCombobox*
	filter() const;

  public slots:
	void
	filter_updated();

  private:
	PortsConnector*	_ports_connector;
	PortsList*		_list;
	UnitsCombobox*	_filter;
};


inline PortsList*
Panel::list() const
{
	return _list;
}


inline UnitsCombobox*
Panel::filter() const
{
	return _filter;
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

#endif

