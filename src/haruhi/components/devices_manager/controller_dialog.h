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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_DIALOG_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_DIALOG_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QDialog>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "controller_item.h"


namespace Haruhi {

namespace DevicesManager {

class ControllerDialog: public QDialog
{
	Q_OBJECT

  public:
	ControllerDialog (QWidget* parent);

	~ControllerDialog();

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
	 * Emitted when user saves controller item.
	 */
	void
	item_configured (ControllerItem*) const;

  protected:
	/**
	 * Filterout Esc events to prevent closing
	 * the dialog widget.
	 */
	void
	keyPressEvent (QKeyEvent*) override;

  private slots:
	void
	update_widgets();

	void
	validate_and_save();

  private:
	Unique<QSpinBox>
	create_channel_spinbox (QWidget* parent);

  private:
	ControllerItem*		_item;

	Unique<QLineEdit>	_name;
	Unique<QPushButton>	_save_button;
	Unique<QCheckBox>	_note_checkbox;
	Unique<QSpinBox>	_note_channel;
	Unique<QCheckBox>	_note_on_velocity_checkbox;
	Unique<QCheckBox>	_note_off_velocity_checkbox;
	Unique<QSpinBox>	_note_velocity_channel;
	Unique<QCheckBox>	_note_pitch_checkbox;
	Unique<QSpinBox>	_note_pitch_channel;
	Unique<QCheckBox>	_controller_checkbox;
	Unique<QSpinBox>	_controller_channel;
	Unique<QSpinBox>	_controller_number;
	Unique<QCheckBox>	_controller_invert;
	Unique<QCheckBox>	_pitchbend_checkbox;
	Unique<QSpinBox>	_pitchbend_channel;
	Unique<QCheckBox>	_channel_pressure_checkbox;
	Unique<QSpinBox>	_channel_pressure_channel;
	Unique<QCheckBox>	_channel_pressure_invert;
	Unique<QCheckBox>	_key_pressure_checkbox;
	Unique<QSpinBox>	_key_pressure_channel;
	Unique<QCheckBox>	_key_pressure_invert;
	Unique<QLabel>		_smoothing_label;
	Unique<QSpinBox>	_smoothing;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

