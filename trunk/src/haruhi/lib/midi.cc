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

// Standard:
#include <cstddef>
#include <string>

// Haruhi:
#include <haruhi/utility/lexical_cast.h>

// Local:
#include "midi.h"


namespace Haruhi {

namespace MIDI {

std::string
key_name (Note key)
{
	std::string r;

	switch (key % 12)
	{
		case 0:		r += "C";			break;
		case 1:		r += "Cis/Des";		break;
		case 2:		r += "D";			break;
		case 3:		r += "Dis/Es";		break;
		case 4:		r += "E";			break;
		case 5:		r += "F";			break;
		case 6:		r += "Fis/Ges";		break;
		case 7:		r += "G";			break;
		case 8:		r += "Gis/Aes";		break;
		case 9:		r += "A";			break;
		case 10:	r += "Ais/Bes";		break;
		case 11:	r += "B";			break;
	}

	r += ' ' + lexical_cast<std::string> (key / 12);
	return r;
}

} // namespace MIDI

} // namespace Haruhi
