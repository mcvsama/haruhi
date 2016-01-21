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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QFont>
#include <QtGui/QApplication>

// Haruhi:
#include <haruhi/application/services.h>

// Local:
#include "resources.h"


namespace Config {

using namespace Haruhi;

int spacing()
{
	return 1.4_screen_mm;
}

int margin()
{
	return 0.8_screen_mm;
}

int window_margin()
{
	return 0.8_screen_mm;
}

int dialog_margin()
{
	return 1.2_screen_mm;
}

} // namespace Config


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


QPixmap
get_icon (QString const& file) noexcept
{
	using namespace Haruhi;

	QPixmap p;
	if (!QPixmapCache::find (file, p))
	{
		p = QPixmap (file).scaled (QSize (4_screen_mm, 4_screen_mm), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		QPixmapCache::insert (file, p);
	}
	return p;
}

} // namespace Resources

