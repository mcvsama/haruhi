/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__PLUGIN__HAS_PRESETS_H__INCLUDED
#define HARUHI__PLUGIN__HAS_PRESETS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QString>
#include <QtXml/QDomNode>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

/**
 * Object inheriting this class supports
 * loading and saving presets.
 */
class HasPresets
{
  public:
	// FIXME change to "= default" in new GCC.
	virtual ~HasPresets() { }

	virtual void
	save_preset (QDomElement&) const = 0;

	virtual void
	load_preset (QDomElement const&) = 0;
};

} // namespace Haruhi

#endif

