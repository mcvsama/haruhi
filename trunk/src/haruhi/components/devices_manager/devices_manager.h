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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__DEVICES_MANAGER_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__DEVICES_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QPushButton>

// Local:
#include "ports_list_view.h"


namespace Haruhi {

class DevicesManager: public QWidget
{
	Q_OBJECT

  public:
	DevicesManager (QWidget* parent);

  public slots:
	/**
	 * Creates new unnamed/unconfigured device
	 * and inserts it into the tree.
	 */
	void
	create_device();

	/**
	 * Creates new unnamed/unconfigured controller
	 * and inserts it into the subtree of currently selected device.
	 * If no device is selected, it does nothing.
	 */
	void
	create_controller();

  private:
	// Widgets:
	QStackedWidget*								_stack;
	QPushButton*								_create_device_button;
	QPushButton*								_create_controller_button;
	QPushButton*								_destroy_input_button;
	DevicesManagerPrivate::PortsListView*		_tree;
//	DevicesManagerPrivate::DeviceDialog*		_device_dialog;
//	DevicesManagerPrivate::ControllerDialog*	_controller_dialog;
};

} // namespace Haruhi

#endif

