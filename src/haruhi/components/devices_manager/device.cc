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

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/lib/midi.h>
#include <haruhi/utility/qdom.h>

// Local:
#include "device.h"


namespace Haruhi {

namespace DevicesManager {

Device::Device (QString const& name):
	_name (name),
	_auto_add (false)
{
	_voice_ids.fill (OmniVoice);
}


bool
Device::operator== (Device const& other) const
{
	return _name == other._name && _controllers == other._controllers && _auto_add == other._auto_add;
}


bool
Device::has_controller (Controller* controller) const
{
	for (Controller const& c: _controllers)
		if (controller == &c)
			return true;
	return false;
}


Device::Controllers::iterator
Device::find_controller (Controller* controller)
{
	for (auto c = _controllers.begin(); c != _controllers.end(); ++c)
		if (controller == &*c)
			return c;
	return _controllers.end();
}


void
Device::save_state (QDomElement& element) const
{
	element.setAttribute ("name", _name);
	element.setAttribute ("auto-add", _auto_add ? "true" : "false");
	for (Controller const& c: _controllers)
	{
		QDomElement e = element.ownerDocument().createElement ("controller");
		c.save_state (e);
		element.appendChild (e);
	}
}


void
Device::load_state (QDomElement const& element)
{
	_name = element.attribute ("name", "<unnamed>");
	_auto_add = element.attribute ("auto-add") == "true";
	_controllers.clear();

	for (QDomElement& e: element)
	{
		if (e.tagName() == "controller")
		{
			_controllers.push_back (Controller());
			_controllers.back().load_state (e);
		}
	}
}

} // namespace DevicesManager

} // namespace Haruhi

