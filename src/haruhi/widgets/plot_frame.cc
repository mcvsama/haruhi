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

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>

// Local:
#include "plot_frame.h"


namespace Haruhi {

PlotFrame::PlotFrame (QWidget* parent):
	QFrame (parent)
{
	setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_layout = new QVBoxLayout (this);
	_layout->setMargin (0);
	_layout->setSpacing (Config::spacing());
}


void
PlotFrame::set_widget (QWidget* widget)
{
	_layout->addWidget (widget);
}

} // namespace Haruhi

