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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__DEVICE_DIALOG_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__DEVICE_DIALOG_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>

// Local:
#include "device_item.h"


namespace Haruhi {

namespace DevicesManager {

class DeviceDialog: public QWidget
{
	Q_OBJECT

  public:
	DeviceDialog (QWidget* parent);

	virtual ~DeviceDialog() { }

	virtual void
	clear();

	virtual void
	from (DeviceItem*);

	virtual void
	apply (DeviceItem*) const;

  private slots:
	void
	update_widgets();

	void
	validate_and_save();

  private:
	DeviceItem*		_item;
	QLineEdit*		_name;
	QPushButton*	_save_button;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

