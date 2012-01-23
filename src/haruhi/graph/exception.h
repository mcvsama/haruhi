/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__GRAPH__EXCEPTION_H__INCLUDED
#define HARUHI__GRAPH__EXCEPTION_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/exception.h>


namespace Haruhi {

class GraphException: public Exception
{
  public:
	explicit GraphException (const char* what, const char* details):
		Exception (what, details)
	{ }
};


class PortException: public GraphException
{
  public:
	explicit PortException (const char* what, const char* details):
		GraphException (what, details)
	{ }
};


class PortIncompatible: public PortException
{
  public:
	explicit PortIncompatible (const char* what, const char* details):
		PortException (what, details)
	{ }
};

} // namespace Haruhi

#endif

