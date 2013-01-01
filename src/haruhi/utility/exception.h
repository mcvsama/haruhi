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

#ifndef HARUHI__UTILITY__EXCEPTION_H__INCLUDED
#define HARUHI__UTILITY__EXCEPTION_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>

// Haruhi:
#include <haruhi/config/all.h>


class Exception: public std::runtime_error
{
  public:
	explicit
	Exception (const char* what, const char* details = ""):
		std::runtime_error (what),
		_details (details)
	{ }

	virtual ~Exception() noexcept = default;

	const char*
	details() const noexcept
	{
		return _details.c_str();
	}

  private:
	std::string _details;
};

#endif

