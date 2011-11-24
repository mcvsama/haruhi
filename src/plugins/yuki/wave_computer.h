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

#ifndef HARUHI__PLUGINS__YUKI__WAVE_COMPUTER_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__WAVE_COMPUTER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/work_performer.h>


namespace Yuki {

/**
 * Adds wave-computing tasks to sessions's low-priority work performer.
 * Signals when work is done.
 */
class WaveComputer
{
};

} // namespace Yuki

#endif

