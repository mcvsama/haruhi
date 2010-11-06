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

#ifndef HARUHI__CORE__NOTIFICATION_H__INCLUDED
#define HARUHI__CORE__NOTIFICATION_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>


namespace Haruhi {

namespace Core {

class Notification
{
  public:
	/**
	 * Creates Notification targeted to all Units.
	 */
	Notification();

	/**
	 * Creates Notification targeted to Units with specified URN.
	 */
	Notification (std::string const& target_urn);

	virtual ~Notification() { }

	/**
	 * \returns	true if Notification is broadcast-type.
	 */
	bool
	broadcast() const { return _broadcast; }

	/**
	 * \returns	target URN if applies.
	 * 			Invalid call if notification is broadcast.
	 */
	std::string
	target_urn() const { return _target_urn; }

  private:
	bool		_broadcast;
	std::string	_target_urn;
};

} // namespace Core

} // namespace Haruhi

#endif

