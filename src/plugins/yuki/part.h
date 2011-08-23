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

#ifndef HARUHI__PLUGINS__YUKI__PART_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>

// Haruhi:
#include <haruhi/config/all.h>


namespace Yuki {

class PartWidget;
class PartManager;
class WaveComputer;

class Part
{
  public:
	PartWidget*
	widget() const { return _widget; }

	/**
	 * Return WaveComputer for this part.
	 * Uses plugin's global WaveComputer.
	 */
	WaveComputer*
	wave_computer() const;

  private:
	PartWidget*		_widget;
	PartManager*	_part_manager;
};

typedef std::list<Part*>  Parts;

} // namespace Yuki

#endif

