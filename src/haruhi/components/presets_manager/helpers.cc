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
#include <QtXml/QDomNode>

// Local:
#include "helpers.h"


namespace Haruhi {

namespace Private = PresetsManagerPrivate;

QDomElement
Private::append_element (QDomElement& subject, QString const& name, QString const& value)
{
	QDomDocument doc = subject.ownerDocument();
	QDomElement element = doc.createElement (name);
	element.appendChild (doc.createTextNode (value));
	subject.appendChild (element);
	return element;
}

} // namespace Haruhi

