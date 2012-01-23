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

#ifndef HARUHI__UTILITY__LEXICAL_CAST_H__INCLUDED
#define HARUHI__UTILITY__LEXICAL_CAST_H__INCLUDED

// Standard:
#include <string>
#include <sstream>

// Haruhi:
#include <haruhi/config/all.h>


/*
 * Simple lexical_cast<> implementation.
 */

namespace detail {

	// Selects character type that is wider:

	template<class Target, class Source>
		struct wider_char
		  { typedef Target  type; };

	template<>
		struct wider_char<char, wchar_t>
		  { typedef wchar_t  type; };

	template<>
		struct wider_char<wchar_t, char>
		  { typedef wchar_t  type; };

	// Selects character type used by stream:

	template<class Stream>
		struct stream_char
		  { typedef char type; };

	template<>
		struct stream_char<wchar_t *>
		  { typedef wchar_t type; };

	template<>
		struct stream_char<const wchar_t *>
		  { typedef wchar_t type; };

	template<>
		struct stream_char<std::string>
		  { typedef char type; };

	template<>
		struct stream_char<std::wstring>
		  { typedef wchar_t type; };

	// Lexical converters:

	template<class Target>
		struct converter
		{
			template<class Stream>
				Target convert (Stream& interpreter)
				{
					Target result;
					interpreter >> result;
					return result;
				}
		};

	template<>
		struct converter<std::string>
		{
			template<class Stream>
				std::string convert (Stream& interpreter)
				  { return interpreter.str(); }
		};
}


template<class Target, class Source>
	Target lexical_cast (Source const& src)
	{
		typedef typename detail::wider_char<
			typename detail::stream_char<Target>::type,
			typename detail::stream_char<Source>::type
		>::type  char_type;

		std::basic_stringstream<char_type>  interpreter;
		std::noskipws (interpreter);
		interpreter << src;

		return detail::converter<Target>().convert (interpreter);
	}


#endif

