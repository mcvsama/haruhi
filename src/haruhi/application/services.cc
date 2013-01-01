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
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/utility/work_performer.h>

// Local:
#include "services.h"


namespace Haruhi {

WorkPerformer*		Services::_hi_priority_work_performer;
WorkPerformer*		Services::_lo_priority_work_performer;
signed int			Services::_detected_cores = -1;
CallOutDispatcher*	Services::_call_out_dispatcher;


void
CallOutDispatcher::customEvent (QEvent* event)
{
	Services::CallOutEvent* coe = dynamic_cast<Services::CallOutEvent*> (event);
	if (coe)
	{
		coe->accept();
		coe->call_out();
	}
}


void
Services::initialize()
{
	_hi_priority_work_performer = new WorkPerformer (detected_cores());
	_lo_priority_work_performer = new WorkPerformer (detected_cores());
	_call_out_dispatcher = new CallOutDispatcher();
}


void
Services::deinitialize()
{
	delete _hi_priority_work_performer;
	delete _lo_priority_work_performer;
	delete _call_out_dispatcher;
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


std::vector<const char*>
Services::features()
{
	std::vector<const char*> features;

#ifdef HARUHI_SSE1
	features.push_back ("SSE1");
#endif
#ifdef HARUHI_SSE2
	features.push_back ("SSE2");
#endif
#ifdef HARUHI_SSE3
	features.push_back ("SSE3");
#endif
#ifdef HARUHI_IEEE754
	features.push_back ("IEEE754");
#endif

	return features;
}


Services::CallOutEvent*
Services::call_out (boost::function<void()> callback)
{
	CallOutEvent* e = new CallOutEvent (callback);
	QApplication::postEvent (_call_out_dispatcher, e);
	return e;
}

} // namespace Haruhi

