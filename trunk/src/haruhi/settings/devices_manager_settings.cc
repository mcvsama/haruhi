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

// Local:
#include "devices_manager_settings.h"


namespace Haruhi {

DevicesManagerSettings::DevicesManagerSettings():
	Module ("devices-manager")
{
}


void
DevicesManagerSettings::save_state (QDomElement& element) const
{
	for (Devices::const_iterator d = _devices.begin(); d != _devices.end(); ++d)
	{
		QDomElement e = d->element().cloneNode (true).toElement();
		e.setTagName ("device");
		element.appendChild (e);
	}
}


void
DevicesManagerSettings::load_state (QDomElement const& element)
{
	_devices.clear();
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (e.isNull())
			continue;
		if (e.tagName() == "device")
			_devices.push_back (Device (e.attribute ("name", "<unknown name>"), e));
	}
}


void
DevicesManagerSettings::save_device (QString const& name, SaveableState const& device)
{
	Device dev (name, QDomElement());
	device.save_state (dev.element());
	_devices.push_back (dev);
}

} // namespace Haruhi
