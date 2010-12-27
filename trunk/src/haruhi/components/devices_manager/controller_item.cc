/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
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
#include <QtGui/QApplication>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/lib/midi.h>

// Local:
#include "controller.h"
#include "controller_item.h"


namespace Haruhi {

namespace DevicesManager {

ControllerItem::ControllerItem (DeviceItem* parent, Controller* controller):
	Item (static_cast<QTreeWidgetItem*> (parent), controller->name()),
	_controller (controller),
	_learning (false)
{
	setIcon (0, Resources::Icons16::event_output_port());
	update_minimum_size();
}


ControllerItem::~ControllerItem()
{
	// Remove itself from tree widget:
	if (parent())
		parent()->takeChild (parent()->indexOfChild (this));
}


void
ControllerItem::set_name (QString const& name)
{
	setText (0, name);
	_controller->set_name (name);
}


void
ControllerItem::learn()
{
	_learning = !_learning;
	if (_learning)
		setIcon (0, Resources::Icons16::colorpicker());
	else
		setIcon (0, Resources::Icons16::event_output_port());
}


void
ControllerItem::stop_learning()
{
	_learning = false;
	// Called from Engine thread, we should not change icon directly:
	QApplication::postEvent (treeWidget(), new Tree::LearnedParams (this));
}


void
ControllerItem::learn_from_event (MIDI::Event const& midi_event)
{
	if (_controller->learn_from_event (midi_event))
		stop_learning();
}

} // namespace DevicesManager

} // namespace Haruhi

