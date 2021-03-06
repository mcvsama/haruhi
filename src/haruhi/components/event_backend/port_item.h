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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__PORT_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__PORT_ITEM_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "tree.h"


namespace Haruhi {

namespace EventBackendImpl {

class Backend;

class PortItem
{
  public:
	PortItem (Backend* backend);

	/**
	 * Updates name of backend ports basing on GUI port name.
	 */
	virtual void
	update_name() = 0;

	/**
	 * Tells whether port has been fully constructed and
	 * may be normally used by backend.
	 */
	bool
	ready() const;

	Backend*
	backend();

  protected:
	/**
	 * Tell that port is ready to use.
	 * Call from derived class constructor.
	 */
	void
	set_ready (bool r);

  private:
	void
	update_minimum_size();

  private:
	Backend*	_backend;
	// Set when port is fully constructed:
	bool		_ready;
};


inline bool
PortItem::ready() const
{
	return _ready;
}


inline Backend*
PortItem::backend()
{
	return _backend;
}


inline void
PortItem::set_ready (bool r)
{
	_ready = r;
}

} // namespace EventBackendImpl

} // namespace Haruhi

#endif

