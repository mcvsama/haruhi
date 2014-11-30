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


class Exception: public std::runtime_error
{
  public:
	explicit
	Exception (const char* what, const char* details = ""):
		std::runtime_error (""),
		_what (what),
		_details (details)
	{ }

	explicit
	Exception (std::string const& what, std::string const& details = ""):
		std::runtime_error (""),
		_what (what),
		_details (details)
	{ }

	virtual ~Exception() noexcept = default;

	const char*
	what() const noexcept
	{
		return _what.c_str();
	}

	const char*
	details() const noexcept
	{
		return _details.c_str();
	}

  private:
	std::string	_what;
	std::string	_details;
};

#endif

