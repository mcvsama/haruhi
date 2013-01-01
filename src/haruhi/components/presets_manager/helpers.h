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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__HELPERS_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__HELPERS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomNode>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

namespace PresetsManagerPrivate {

/**
 * DOM helper function.
 */
QDomElement
append_element (QDomElement& subject, QString const& name, QString const& value);

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

