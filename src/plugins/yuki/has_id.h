
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

#ifndef HARUHI__PLUGINS__YUKI__HAS_ID_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__HAS_ID_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>


namespace Yuki {

class HasID
{
  public:
	HasID();

	/**
	 * Return part ID. Used by user to distinguish between different parts.
	 */
	unsigned int
	id() const;

	/**
	 * Set part ID.
	 */
	void
	set_id (unsigned int id);

  private:
	unsigned int _id;
};


inline
HasID::HasID():
	_id (0)
{ }


inline unsigned int
HasID::id() const
{
	return _id;
}


inline void
HasID::set_id (unsigned int id)
{
	_id = id;
}

} // namespace Yuki

#endif

