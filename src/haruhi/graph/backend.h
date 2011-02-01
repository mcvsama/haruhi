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

#ifndef HARUHI__GRAPH__BACKEND_H__INCLUDED
#define HARUHI__GRAPH__BACKEND_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Local:
#include "unit.h"


namespace Haruhi {

class Backend: public Unit
{
  public:
	Backend (std::string const& urn, std::string const& title, int id);

	/**
	 * Connects backend object to system backend.
	 */
	virtual void
	connect() = 0;

	/**
	 * Disconnects backend from system.
	 */
	virtual void
	disconnect() = 0;

	/**
	 * \returns	true if backend is connected.
	 */
	virtual bool
	connected() const = 0;
};

} // namespace Haruhi

#endif
