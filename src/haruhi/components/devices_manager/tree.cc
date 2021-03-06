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
#include <algorithm>
#include <set>

// Qt:
#include <QTreeWidget>
#include <QHeaderView>

// Haruhi:
#include <haruhi/application/haruhi.h>

// Local:
#include "tree.h"
#include "device_item.h"
#include "controller_item.h"
#include "model.h"


namespace Haruhi {

namespace DevicesManager {

Tree::Tree (QWidget* parent, Model* model):
	QTreeWidget (parent),
	_model (model)
{
	header()->setSectionsClickable (false);
	header()->setSectionResizeMode (QHeaderView::Stretch);
	setMinimumWidth (160);
	setSortingEnabled (true);
	sortByColumn (0, Qt::AscendingOrder);
	setSelectionMode (QTreeWidget::SingleSelection);
	setRootIsDecorated (true);
	setAllColumnsShowFocus (true);
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
	setContextMenuPolicy (Qt::CustomContextMenu);
	setHeaderLabel ("Device templates");
	setIconSize (Resources::Icons16::haruhi().size());
	_model->on_change.connect (this, &Tree::read_model);
}


void
Tree::read_model()
{
	typedef std::set<Device*>		DevicePtrs;
	typedef std::set<Controller*>	ControllerPtrs;

	QTreeWidgetItem* last_added_item = 0;

	// Create set of Device pointers:
	DevicePtrs device_ptrs;
	for (Device& d: _model->devices())
		device_ptrs.insert (&d);

	// Check devices:
	int dn = invisibleRootItem()->childCount();
	for (int di = 0; di < dn; ++di)
	{
		auto device_item = dynamic_cast<DeviceItem*> (invisibleRootItem()->child (di));
		if (!device_item)
			continue;

		auto device = device_item->device();
		device_ptrs.erase (device);
		if (!_model->has_device (device))
		{
			device_item->parent()->removeChild (device_item);
			delete device_item;
		}
		else
		{
			// Create set of Controller pointers:
			ControllerPtrs controller_ptrs;
			for (Controller& c: device->controllers())
				controller_ptrs.insert (&c);

			// Check controllers:
			int cn = device_item->childCount();
			for (int ci = 0; ci < cn; ++ci)
			{
				auto controller_item = dynamic_cast<ControllerItem*> (device_item->child (ci));
				if (!controller_item)
					continue;

				auto controller = controller_item->controller();
				controller_ptrs.erase (controller);
				if (!device->has_controller (controller))
				{
					controller_item->parent()->removeChild (controller_item);
					delete controller_item;
				}
			}

			// Controllers left in controller_ptrs are new controllers, need to be added:
			for (Controller* cptr: controller_ptrs)
				last_added_item = device_item->create_controller_item (cptr);
		}
	}

	// Devices left in device_ptrs are new devices, need to be added:
	for (Device* dptr: device_ptrs)
	{
		DeviceItem* device_item = create_device_item (dptr);
		create_controller_items (device_item);
		last_added_item = device_item;
	}

	if (last_added_item)
		setCurrentItem (last_added_item);
}


void
Tree::create_device()
{
	Device dev ("<unnamed device>");
	_model->devices().push_back (dev);
	_model->changed();
}


void
Tree::create_controller()
{
	auto sel = selected_item();
	if (sel != 0)
	{
		auto device_item = dynamic_cast<DeviceItem*> (sel);
		if (device_item == 0)
			device_item = dynamic_cast<DeviceItem*> (sel->parent());
		if (device_item != 0)
		{
			auto device = device_item->device();
			Controller contr ("<unnamed controller>");
			device->controllers().push_back (contr);
			_model->changed();
		}
	}
}


void
Tree::destroy_selected_item()
{
	auto item = selected_item();
	if (item)
	{
		DeviceItem* device_item;
		ControllerItem* controller_item;

		if ((device_item = dynamic_cast<DeviceItem*> (item)))
		{
			Model::Devices::iterator it = _model->find_device (device_item->device());
			assert (it != _model->devices().end());
			_model->devices().erase (it);
			if (device_item->parent())
				device_item->parent()->removeChild (device_item);
			delete device_item;
			_model->changed();
		}
		else if ((controller_item = dynamic_cast<ControllerItem*> (item)))
		{
			device_item = dynamic_cast<DeviceItem*> (controller_item->parent());
			assert (device_item != 0);
			auto device = device_item->device();
			auto it = device->find_controller (controller_item->controller());
			assert (it != device->controllers().end());
			device->controllers().erase (it);
			device_item->removeChild (controller_item);
			delete controller_item;
			_model->changed();
		}
	}
}


QTreeWidgetItem*
Tree::selected_item() const
{
	auto list = selectedItems();
	return list.empty() ? 0 : list.front();
}


DeviceItem*
Tree::create_device_item (Device* device)
{
	return new DeviceItem (this, device);
}


void
Tree::create_controller_items (DeviceItem* device_item)
{
	auto& controllers = device_item->device()->controllers();
	for (Controller& c: controllers)
		device_item->create_controller_item (&c);
}


void
Tree::customEvent (QEvent* event)
{
	auto lp = dynamic_cast<LearnedParams*> (event);
	if (lp)
	{
		lp->item->treeWidget()->clearSelection();
		// Force update of editor dialog:
		lp->item->setSelected (false);
		lp->item->setSelected (true);
		// Reset icon:
		lp->item->setIcon (0, Resources::Icons16::event_output_port());
	}
}

} // namespace DevicesManager

} // namespace Haruhi

