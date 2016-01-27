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
#include <QLayout>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/qdom.h>

// Local:
#include "plugin.h"
#include "part_manager.h"
#include "part_manager_widget.h"


namespace Yuki {

Plugin::Plugin (std::string const& urn, std::string const& title, int id, QWidget* parent):
	Haruhi::Plugin (urn, title, id, parent)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_part_manager = std::make_unique<PartManager> (this);
	_part_manager_widget = std::make_unique<PartManagerWidget> (this, _part_manager.get());
	_part_manager->set_widget (_part_manager_widget.get());

	auto layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addWidget (_part_manager_widget.get());
}


void
Plugin::registered()
{
	graph_updated(); // Initially resize buffers.
	enable();
}


void
Plugin::unregistered()
{
	panic();
}


void
Plugin::process()
{
	sync_inputs();
	clear_outputs();
	_part_manager->process();
}


void
Plugin::panic()
{
	_part_manager->panic();
}


void
Plugin::graph_updated()
{
	Unit::graph_updated();
	_part_manager->graph_updated();
}


int
Plugin::voices_number() const
{
	return _part_manager->voices_number();
}


void
Plugin::set_unit_bay (Haruhi::UnitBay* unit_bay)
{
	UnitBayAware::set_unit_bay (unit_bay);
	_part_manager_widget->unit_bay_assigned();
}


void
Plugin::save_state (QDomElement& element) const
{
	QDomElement state = element.ownerDocument().createElement ("state");
	_part_manager->save_state (state);
	element.appendChild (state);
}


void
Plugin::load_state (QDomElement const& element)
{
	disable();
	for (QDomElement& e: element)
	{
		if (e.tagName() == "state")
		{
			_part_manager->load_state (e);
			break;
		}
	}
	enable();
}

} // namespace Yuki

