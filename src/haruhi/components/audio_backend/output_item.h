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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__OUTPUT_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__OUTPUT_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/dsp/smoother.h>

// Local:
#include "port_item.h"


namespace Haruhi {

namespace AudioBackendImpl {

class OutputItem: public PortItem
{
  public:
	OutputItem (Tree* parent, QString const& name);

	virtual ~OutputItem();

	void
	configure();

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);
};

} // namespace AudioBackendImpl

} // namespace Haruhi

#endif

