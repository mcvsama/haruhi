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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__TREE_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__TREE_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QEvent>
#include <QtGui/QTreeWidget>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/signal.h>

// Local:
#include "device.h"


namespace Haruhi {

namespace DevicesManager {

class DeviceItem;
class Model;

class Tree:
	public QTreeWidget,
	public Signal::Receiver
{
	Q_OBJECT

  public:
	/**
	 * Due to bugs in Qt's MOC (Meta Object Compiler) we cannot create
	 * PortItem that is QObject and Q_OBJECT in non-default namespace,
	 * therefore we can't send events to PortItems directly. We must
	 * do it in Tree.
	 *
	 * This event object is sent from QTreeWidgetItem to its QTreeWidget
	 * so that it will change item's icon from within UI thread.
	 */
	class LearnedParams: public QEvent
	{
	  public:
		LearnedParams (QTreeWidgetItem* item);

	  public:
		QTreeWidgetItem* item;
	};

  public:
	/**
	 * \param	parent Parent widget.
	 * \param	model Model object that will be manipulated by this tree.
	 */
	Tree (QWidget* parent, Model* model);

	/**
	 * Reads model and populates tree according to it.
	 * This method Will not remove items that weren't removed from model.
	 */
	void
	read_model();

  public slots:
	/**
	 * Creates unnamed device item.
	 * Updates model.
	 */
	void
	create_device();

	/**
	 * Creates unnamed controller under selected device.
	 * Does nothing, if no device item is selected.
	 * Updates model.
	 */
	void
	create_controller();

	/**
	 * Destroys selected item (device or controller).
	 * Does nothing, if no item is selected.
	 * Updates model.
	 */
	void
	destroy_selected_item();

	/**
	 * Returns selected item or 0, if none selected.
	 */
	QTreeWidgetItem*
	selected_item() const;

  protected:
	/**
	 * Returns pointer to external Model.
	 */
	Model*
	model() const;

	/**
	 * Allocates DeviceItem that will be used as child for this PortsList.
	 * Only allocates DeviceItem, does not descend into child Controllers.
	 * Use create_controller_items() to also allocate ControllerItems.
	 *
	 * \param	device Device object associated with newly created item.
	 * 			Item will not take ownership of the object.
	 */
	virtual DeviceItem*
	create_device_item (Device* device);

	/**
	 * Iterates device's controllers list and creates ControllerItems
	 * for given DeviceItem. ControllerItems are allocated using device_item->create_controller_item().
	 */
	void
	create_controller_items (DeviceItem* device_item);

  protected:
	void
	customEvent (QEvent*) override;

  private:
	Model* _model;
};


inline
Tree::LearnedParams::LearnedParams (QTreeWidgetItem* item):
	QEvent (QEvent::User),
	item (item)
{ }


inline Model*
Tree::model() const
{
	return _model;
}

} // namespace DevicesManager

} // namespace Haruhi

#endif

