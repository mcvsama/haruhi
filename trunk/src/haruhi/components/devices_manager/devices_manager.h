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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__DEVICES_MANAGER__DEVICES_MANAGER_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__DEVICES_MANAGER__DEVICES_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QStackedWidget>


namespace Haruhi {

class DevicesManager: public QWidget
{
	Q_OBJECT

  public:
	DevicesManager (QWidget* parent);

  private:
	QStackedWidget*								_stack;
//	DevicesManagerPrivate::DeviceDialog*		_device_dialog;
//	DevicesManagerPrivate::ControllerDialog*	_controller_dialog;
//	DevicesManagerPrivate::PortsListView*		_ports_list;
};

} // namespace Haruhi

#endif

