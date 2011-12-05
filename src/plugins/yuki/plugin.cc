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
#include <QtGui/QLayout>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "plugin.h"
#include "part_manager.h"
#include "part_manager_widget.h"


namespace Yuki {

Plugin::Plugin (std::string const& urn, std::string const& title, int id, QWidget* parent):
	Haruhi::Plugin (urn, title, id, parent)
{
	_part_manager = new PartManager (this);
	_part_manager_widget = new PartManagerWidget (this, _part_manager);
	_part_manager->set_widget (_part_manager_widget);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (_part_manager_widget);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}


Plugin::~Plugin()
{
	delete _part_manager_widget;
	delete _part_manager;
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

} // namespace Yuki

