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

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/utility/work_performer.h>

// Local:
#include "services.h"


namespace Haruhi {

WorkPerformer* Services::_hi_priority_work_performer;
WorkPerformer* Services::_lo_priority_work_performer;


void
Services::initialize()
{
	_hi_priority_work_performer = new WorkPerformer (Haruhi::Haruhi::detected_cores());
	_lo_priority_work_performer = new WorkPerformer (Haruhi::Haruhi::detected_cores());
}


void
Services::deinitialize()
{
	delete _hi_priority_work_performer;
	delete _lo_priority_work_performer;
}

} // namespace Haruhi

