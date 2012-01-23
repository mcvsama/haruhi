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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__DEVICE_DIALOG_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__DEVICE_DIALOG_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdint.h>

// Qt:
#include <QtGui/QDialog>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QCheckBox>
#include <QtGui/QKeyEvent>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "device_item.h"


namespace Haruhi {

namespace DevicesManager {

class DeviceDialog: public QDialog
{
	Q_OBJECT

  public:
	typedef uint32_t Flags;

	// Flags:
	enum {
		DisplayAutoAdd	= 0x01,
	};

  public:
	DeviceDialog (QWidget* parent, Flags flags = 0);

	virtual ~DeviceDialog() { }

	virtual void
	clear();

	virtual void
	from (DeviceItem*);

	virtual void
	apply (DeviceItem*) const;

  signals:
	/**
	 * Emited when user saves device item.
	 */
	void
	item_configured (DeviceItem*) const;

  protected:
	/**
	 * Filterout Esc events to prevent closing
	 * the dialog widget.
	 */
	void
	keyPressEvent (QKeyEvent*);

  private slots:
	void
	update_widgets();

	void
	validate_and_save();

  private:
	Flags			_flags;
	DeviceItem*		_item;
	QLineEdit*		_name;
	QPushButton*	_save_button;
	QCheckBox*		_auto_add_checkbox;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

