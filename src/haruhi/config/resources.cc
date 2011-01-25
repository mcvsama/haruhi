/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
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

// Qt:
#include <QtGui/QFont>
#include <QtGui/QApplication>

// Local:
#include "resources.h"


namespace Resources {

QFont&
small_font()
{
	static QFont sf;
	static bool sf_initialized = false;

	if (!sf_initialized)
	{
		sf = QApplication::font();
		sf.setPointSize (8);
	}

	return sf;
}

} // namespace Resources

