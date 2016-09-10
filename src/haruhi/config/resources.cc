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
#include <QFont>
#include <QApplication>
#include <QPixmap>
#include <QPixmapCache>
#include <QPainter>
#include <QSvgRenderer>

// Haruhi:
#include <haruhi/application/services.h>

// Local:
#include "resources.h"


namespace Config {

using namespace Haruhi::ScreenLiterals;

int spacing()
{
	return 0.27_em;
}

int margin()
{
	return 0.27_em;
}

int window_margin()
{
	return 0.33_em;
}

int dialog_margin()
{
	return 0.33_em;
}

} // namespace Config


namespace Resources {

using namespace Haruhi::ScreenLiterals;


QFont&
small_font()
{
	static QFont sf;
	static bool sf_initialized = false;

	if (!sf_initialized)
	{
		sf = QApplication::font();
		sf.setPixelSize (0.76_em);
	}

	return sf;
}


QPixmap
get_png_icon (QString const& png_file) noexcept
{
	QPixmap pixmap;
	QString key = "icon.png." + png_file;

	if (!QPixmapCache::find (key, pixmap))
	{
		pixmap = QPixmap (png_file);
		QPixmapCache::insert (key, pixmap);
	}

	return pixmap;
}


QPixmap
get_svg_icon (QString const& svg_file) noexcept
{
	QPixmap pixmap;
	QString key = "icon.svg." + svg_file;

	if (!QPixmapCache::find (key, pixmap))
	{
		QSvgRenderer svg (svg_file);
		pixmap = QPixmap (QSize (1.6_em, 1.6_em));
		pixmap.fill (Qt::transparent);
		QPainter painter (&pixmap);
		svg.render (&painter);
		QPixmapCache::insert (key, pixmap);
	}

	return pixmap;
}

} // namespace Resources

