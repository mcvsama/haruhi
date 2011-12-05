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

// Standard:
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/utility/work_performer.h>

// Local:
#include "services.h"


namespace Haruhi {

WorkPerformer*	Services::_hi_priority_work_performer;
WorkPerformer*	Services::_lo_priority_work_performer;
signed int		Services::_detected_cores;


void
Services::initialize()
{
	_hi_priority_work_performer = new WorkPerformer (detected_cores());
	_lo_priority_work_performer = new WorkPerformer (detected_cores());
}


void
Services::deinitialize()
{
	delete _hi_priority_work_performer;
	delete _lo_priority_work_performer;
}


unsigned int
Services::detected_cores()
{
	if (_detected_cores != -1)
		return _detected_cores;

	_detected_cores = 0;
	std::ifstream cpuinfo ("/proc/cpuinfo");
	std::string line;
	while (cpuinfo.good())
	{
		std::getline (cpuinfo, line);
		std::istringstream s (line);
		std::string name, colon;
		s >> name >> colon;
		if (name == "processor" && colon == ":")
			++_detected_cores;
	}
	return _detected_cores;
}

} // namespace Haruhi

