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
#include <thread>
#include <vector>
#include <experimental/optional>

// Qt:
#include <QDesktopWidget>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/utility/work_performer.h>

// Local:
#include "services.h"


namespace Haruhi {

Unique<WorkPerformer>		Services::_hi_priority_work_performer;
Unique<WorkPerformer>		Services::_lo_priority_work_performer;
Unique<CallOutDispatcher>	Services::_call_out_dispatcher;


void
CallOutDispatcher::customEvent (QEvent* event)
{
	if (auto coe = dynamic_cast<Services::CallOutEvent*> (event))
	{
		coe->accept();
		coe->call_out();
	}
}


void
Services::initialize()
{
	_hi_priority_work_performer = std::make_unique<WorkPerformer> (std::thread::hardware_concurrency());
	_lo_priority_work_performer = std::make_unique<WorkPerformer> (std::thread::hardware_concurrency());
	_call_out_dispatcher = std::make_unique<CallOutDispatcher>();
}


void
Services::deinitialize()
{
	_hi_priority_work_performer.reset();
	_lo_priority_work_performer.reset();
	_call_out_dispatcher.reset();
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
Services::call_out (std::function<void()> callback)
{
	CallOutEvent* e = new CallOutEvent (callback);
	QApplication::postEvent (_call_out_dispatcher.get(), e);
	return e;
}


float
Services::master_ui_scaling_factor()
{
	return 1.0;
}


float
Services::x_pixels_per_point()
{
	// 1 point is 1/72 of an inch:
	return QApplication::desktop()->physicalDpiX() / 72.0f * master_ui_scaling_factor();
}


float
Services::y_pixels_per_point()
{
	// 1 point is 1/72 of an inch:
	return QApplication::desktop()->physicalDpiY() / 72.0f * master_ui_scaling_factor();
}


float
Services::y_pixels_per_em()
{
	static std::experimental::optional<float> value;

	if (!value)
	{
		auto f = QApplication::font();
		value = f.pointSize() * y_pixels_per_point();
	}

	return *value;
}

} // namespace Haruhi

