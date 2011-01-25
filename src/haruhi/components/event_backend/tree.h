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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__TREE_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__TREE_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QEvent>
#include <QtGui/QTreeWidget>

// Haruhi:
#include <haruhi/utility/saveable_state.h>
#include <haruhi/components/devices_manager/tree.h>
#include <haruhi/components/devices_manager/model.h>


namespace Haruhi {

namespace EventBackendImpl {

using DevicesManager::DeviceItem;
class Backend;

class Tree: public DevicesManager::Tree
{
  public:
	/**
	 * \param	parent Parent widget.
	 * \param	backend	Backend object that owns this widget.
	 * \param	model External Model object, that will be manipulated.
	 */
	Tree (QWidget* parent, Backend* backend, DevicesManager::Model* model);

	/**
	 * Returns pointer to external Backend object.
	 */
	Backend*
	backend() const { return _backend; }

	/**
	 * Overridden from DevicesManager::Tree.
	 */
	DeviceItem*
	create_device_item (DevicesManager::Device* device);

  private:
	Backend* _backend;
};

} // namespace EventBackendImpl

} // namespace Haruhi

#endif

