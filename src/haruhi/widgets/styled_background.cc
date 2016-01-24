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
#include <QtGui/QWidget>
#include <QtGui/QLayout>

// Haruhi:
#include <haruhi/application/services.h>

// Local:
#include "styled_background.h"


namespace Haruhi {

using namespace Haruhi::ScreenLiterals;


StyledBackground::StyledBackground (QWidget* child, QWidget* parent, int margin):
	QWidget (parent)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setAutoFillBackground (true);
	setForegroundRole (QPalette::Text);
	setBackgroundRole (QPalette::Dark);
	setContentsMargins (0, 1, 0, 0);

	QHBoxLayout* layout = new QHBoxLayout (this);
	layout->setMargin (margin + 0.3_screen_mm);
	layout->setSpacing (0);
	layout->addItem (new QSpacerItem (0.45_screen_mm, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
	layout->addWidget (child);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
}

} // namespace Haruhi

