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

#ifndef HARUHI__UTILITY__LITERALS_H__INCLUDED
#define HARUHI__UTILITY__LITERALS_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Qt:
#include <QtCore/QString>

// Haruhi:
#include <haruhi/config/all.h>


inline std::string
operator"" _str (const char* string, size_t)
{
	return std::string (string);
}


inline QString
operator"" _qstr (const char* string, size_t)
{
	return QString (string);
}

#endif

