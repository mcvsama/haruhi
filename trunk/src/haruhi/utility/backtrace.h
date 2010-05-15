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

#ifndef HARUHI__UTILITY__BACKTRACE_H__INCLUDED
#define HARUHI__UTILITY__BACKTRACE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdlib>
#include <string>
#include <iostream>
#include <iomanip>
#include <cxxabi.h>

// System:
#include <execinfo.h>


#define SANITY_CHECK(x) \
	if (!x) \
	{ \
		std::clog << "Error: sanity check [" #x "] failed at " << __FILE__ << ":" << __LINE__ << " in " << __func__ << "()\n"; \
		Backtrace::clog(); \
	}


class Backtrace
{
  public:
	static void
	clog()
	{
		std::clog << "Stacktrace:" << std::endl;
		const int MAX = 64;
		void* buffer[MAX];
		int r = ::backtrace (buffer, MAX);
		char** symbols = ::backtrace_symbols (buffer, r);
		for (int i = 1; i < r; ++i)
		{
			std::string symbol (symbols[i]);
			std::string::size_type a = symbol.find ('(') + 1;
			std::string::size_type b = symbol.find ('+', a);
			if (b == std::string::npos)
				b = symbol.find (')');
			std::string name (symbol.substr (a, b - a));

			// Demangle name:
			int demangle_status = 0;
			std::size_t demangled_max_size = 256;
			char demangled_name[demangled_max_size];
			abi::__cxa_demangle (name.c_str(), demangled_name, &demangled_max_size, &demangle_status);

			std::clog << "  " << std::setw (4) << i << ". " << symbol.substr (0, a-1) << ": ";
			std::clog << ((demangle_status == 0) ? demangled_name : name) << std::endl;
		}
		::free (symbols);
	}
};

#endif

