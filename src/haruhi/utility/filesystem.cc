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

// Standard:
#include <cstddef>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <iostream>

// System:
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>


int
file_size (std::string file_name)
{
	std::ifstream f;
	f.open (file_name.c_str(), std::ios_base::binary | std::ios_base::in);
	if (!f.good() || f.eof() || !f.is_open())
		return 0;
	f.seekg (0, std::ios_base::beg);
	std::ifstream::pos_type begin_pos = f.tellg();
	f.seekg (0, std::ios_base::end);
	return static_cast<int> (f.tellg() - begin_pos);
}


void
mkpath (std::string const& path, mode_t mode)
{
	std::stringstream stream (path);
	std::string x;
	std::vector<std::string> dirs;

	while (std::getline (stream, x, '/'))
		dirs.push_back (x);
	x.clear();
	for (std::vector<std::string>::iterator d = dirs.begin(); d != dirs.end(); ++d)
	{
		x += "/" + *d;
		if (::mkdir (x.c_str(), mode) == -1)
		{
			if (errno == EEXIST)
				continue;
			std::cerr << "Warning: failed to make directory '" << x << "' as requested by mkpath()." << std::endl;
			break;
		}
	}
}

