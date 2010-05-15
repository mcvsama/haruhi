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

// Qt:
#include <QtGui/QLayout>

// Haruhi:
#include <haruhi/config.h>

// Local:
#include "panel.h"
#include "ports_connector.h"
#include "units_combobox.h"
#include "ports_list.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

Panel::Panel (Core::Port::Direction type, PortsConnector* ports_connector, QWidget* parent):
	QWidget (parent),
	_ports_connector (ports_connector)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_layout = new QVBoxLayout (this, 0, Config::spacing);

		_filter = new UnitsCombobox (ports_connector->unit_bay(), type, this);
		_list = new PortsList (type, this, ports_connector, this);

		QObject::connect (_filter, SIGNAL (activated (int)), this, SLOT (filter_updated()));

	_layout->addWidget (_filter);
	_layout->addWidget (_list);
}


void
Panel::filter_updated()
{
	_list->set_filter (_filter->unit());
	_ports_connector->connector()->update();
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

