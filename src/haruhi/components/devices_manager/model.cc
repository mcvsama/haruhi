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
#include <QtGui/QTreeWidget>
#include <QtGui/QHeaderView>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/qdom.h>

// Local:
#include "model.h"


namespace Haruhi {

namespace DevicesManager {

Model::Model()
{
}


void
Model::changed()
{
	on_change();
}


bool
Model::has_device (Device* device) const
{
	for (Device const& d: _devices)
		if (device == &d)
			return true;
	return false;
}


bool
Model::has_device_named (QString const& name) const
{
	for (Device const& d: _devices)
		if (d.name() == name)
			return true;
	return false;
}


Model::Devices::iterator
Model::find_device (Device* device)
{
	for (auto d = _devices.begin(); d != _devices.end(); ++d)
		if (device == &*d)
			return d;
	return _devices.end();
}


void
Model::save_state (QDomElement& element) const
{
	for (Device const& d: _devices)
	{
		QDomElement e = element.ownerDocument().createElement ("device");
		d.save_state (e);
		element.appendChild (e);
	}
}


void
Model::load_state (QDomElement const& element)
{
	_devices.clear();
	for (QDomElement& e: element)
	{
		if (e.tagName() == "device")
		{
			_devices.push_back (Device());
			_devices.back().load_state (e);
		}
	}
	changed();
}

} // namespace DevicesManager

} // namespace Haruhi

