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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_DIALOG_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_DIALOG_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>

// Local:
#include "controller_item.h"


namespace Haruhi {

namespace DevicesManager {

class ControllerDialog: public QDialog
{
	Q_OBJECT

  public:
	ControllerDialog (QWidget* parent);

	virtual ~ControllerDialog() { }

	ControllerItem*
	item() const;

	virtual void
	clear();

	virtual void
	from (ControllerItem*);

	virtual void
	apply (ControllerItem*) const;

  signals:
	/**
	 * Emited when user saves controller item.
	 */
	void
	item_configured (ControllerItem*) const;

  private slots:
	void
	update_widgets();

	void
	validate_and_save();

  private:
	QSpinBox*
	create_channel_spinbox (QWidget* parent);

  private:
	ControllerItem*		_item;
	QLineEdit*			_name;
	QPushButton*		_save_button;
	QCheckBox*			_note_checkbox;
	QSpinBox*			_note_channel;
	QCheckBox*			_controller_checkbox;
	QSpinBox*			_controller_channel;
	QSpinBox*			_controller_number;
	QCheckBox*			_controller_invert;
	QCheckBox*			_pitchbend_checkbox;
	QSpinBox*			_pitchbend_channel;
	QCheckBox*			_channel_pressure_checkbox;
	QSpinBox*			_channel_pressure_channel;
	QCheckBox*			_channel_pressure_invert;
	QCheckBox*			_key_pressure_checkbox;
	QSpinBox*			_key_pressure_channel;
	QCheckBox*			_key_pressure_invert;
	QLabel*				_smoothing_label;
	QSpinBox*			_smoothing;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

