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

// Qt:
#include <QLayout>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "panel.h"
#include "ports_connector.h"
#include "units_combobox.h"
#include "ports_list.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

Panel::Panel (Port::Direction type, PortsConnector* ports_connector, QWidget* parent):
	QWidget (parent),
	_ports_connector (ports_connector)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_filter = std::make_unique<UnitsCombobox> (ports_connector, type, this);
	QObject::connect (_filter.get(), SIGNAL (activated (int)), this, SLOT (filter_updated()));

	_list = std::make_unique<PortsList> (type, this, ports_connector, this);

	auto layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addWidget (_filter.get());
	layout->addWidget (_list.get());
}


void
Panel::filter_updated()
{
	_list->set_filter (_filter->unit());
	_ports_connector->connector()->update();
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

