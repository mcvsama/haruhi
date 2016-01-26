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
#include <QMessageBox>
#include <QTextDocument>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/backtrace.h>

// Local:
#include "settings.h"


namespace Haruhi {

namespace DevicesManager {

Settings::Settings():
	Module ("devices-manager")
{
}


void
Settings::save_state (QDomElement& element) const
{
	_model.save_state (element);
}


void
Settings::load_state (QDomElement const& element)
{
	_model.on_change.disconnect (this, &Settings::save_after_change);
	_model.load_state (element);
	_model.on_change.connect (this, &Settings::save_after_change);
}


void
Settings::add_device (Device const& p_device)
{
	Device device (p_device);
	// Check for name collisions, possibly appending /0, /1, to the device name:
	if (_model.has_device_named (device.name()))
	{
		for (int i = 1; i < 999; ++i)
		{
			QString new_name = device.name() + QString ("/%1").arg (i);
			if (!_model.has_device_named (new_name))
			{
				device.set_name (new_name);
				break;
			}
		}
	}

	_model.devices().push_back (device);
	_model.changed();

	QMessageBox::information (0, "Template saved", QString ("Device template saved as: <b>%1</b>").arg (device.name().toHtmlEscaped()));
}

} // namespace DevicesManager

} // namespace Haruhi

