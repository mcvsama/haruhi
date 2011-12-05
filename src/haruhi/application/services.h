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

#ifndef HARUHI__GRAPH__SERVICES_H__INCLUDED
#define HARUHI__GRAPH__SERVICES_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/work_performer.h>


namespace Haruhi {

class Services
{
  public:
	static void
	initialize();

	static void
	deinitialize();

	/**
	 * Return RT-prioritized work performer.
	 */
	static WorkPerformer*
	hi_priority_work_performer() { return _hi_priority_work_performer; }

	/**
	 * Return normal work performer.
	 */
	static WorkPerformer*
	lo_priority_work_performer() { return _lo_priority_work_performer; }

	/**
	 * Return number of detected processor cores.
	 */
	static unsigned int
	detected_cores();

  private:
	static WorkPerformer*	_hi_priority_work_performer;
	static WorkPerformer*	_lo_priority_work_performer;
	static signed int		_detected_cores;
};

} // namespace Haruhi

#endif

