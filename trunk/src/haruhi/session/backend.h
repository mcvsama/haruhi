/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__SESSION__BACKEND_H__INCLUDED
#define HARUHI__SESSION__BACKEND_H__INCLUDED

// Standard:
#include <cstddef>


namespace Haruhi {

class Backend
{
  public:
	/**
	 * Connects object to backend.
	 */
	virtual void
	connect() = 0;

	/**
	 * Disconnects from backend.
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

