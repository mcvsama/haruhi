/* vim:ts=4
 *
 * Copyleft 2010  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__HARUHI_TCC__INCLUDED
#define HARUHI__HARUHI_TCC__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/utility/lexical_cast.h>


template<class T>
	T
	Config::option (QString const& option_name)
	{
		StringsMap::const_iterator f = _map.find (option_name);
		if (f != _map.end())
			return lexical_cast<T> (static_cast<const char*> (f->second.toUtf8()));
		return T();
	}

#endif

