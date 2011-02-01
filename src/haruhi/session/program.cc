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

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "program.h"


namespace Haruhi {

Program::Program (Session* session, QWidget* parent):
	QWidget (parent)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_layout = new QVBoxLayout (this, 0, Config::Spacing);

		_patch = new Patch (session, "Patch", this);

	_layout->addWidget (_patch);
}


Program::~Program()
{
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
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
			if (e.tagName() == "patch")
				_patch->load_state (e);
	}
}

} // namespace Haruhi

