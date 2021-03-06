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
#include <haruhi/utility/qdom.h>

// Qt:
#include <QLayout>

// Local:
#include "program.h"


namespace Haruhi {

Program::Program (Session* session, QWidget* parent):
	QWidget (parent)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_patch = std::make_unique<Patch> (session, "Patch", this);

	auto layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addWidget (_patch.get());
}


void
Program::show_prev_plugin()
{
	_patch->show_prev_tab();
}


void
Program::show_next_plugin()
{
	_patch->show_next_tab();
}


void
Program::save_state (QDomElement& element) const
{
	QDomElement patch = element.ownerDocument().createElement ("patch");
	_patch->save_state (patch);

	element.appendChild (patch);
}


void
Program::load_state (QDomElement const& element)
{
	for (QDomElement& e: element)
	{
		if (e.tagName() == "patch")
			_patch->load_state (e);
	}
}

} // namespace Haruhi

