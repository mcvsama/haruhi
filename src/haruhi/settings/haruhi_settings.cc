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

// Haruhi:
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/qdom.h>

// Local:
#include "haruhi_settings.h"


namespace Haruhi {

HaruhiSettings::HaruhiSettings():
	Module ("haruhi"),
	_engine_thread_priority (50),
	_level_meter_fps (30)
{
}


void
HaruhiSettings::load_state (QDomElement const& element)
{
	for (QDomElement& e: element)
	{
		if (e.tagName() == "engine-thread-priority")
			_engine_thread_priority = e.text().toInt();
		else if (e.tagName() == "level-meter-fps")
			_level_meter_fps = e.text().toInt();
	}

	clamp (_engine_thread_priority, 1, 99);
	clamp (_level_meter_fps, 10, 50);
}


void
HaruhiSettings::save_state (QDomElement& element) const
{
	QDomElement par_engine_thread_priority = element.ownerDocument().createElement ("engine-thread-priority");
	par_engine_thread_priority.appendChild (element.ownerDocument().createTextNode (QString::number (_engine_thread_priority)));

	QDomElement par_level_meter_fps = element.ownerDocument().createElement ("level-meter-fps");
	par_level_meter_fps.appendChild (element.ownerDocument().createTextNode (QString::number (_level_meter_fps)));

	element.appendChild (par_engine_thread_priority);
	element.appendChild (par_level_meter_fps);
}

} // namespace Haruhi
