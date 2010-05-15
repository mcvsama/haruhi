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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__INTERNAL_INPUT_DIALOG_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__INTERNAL_INPUT_DIALOG_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>

// Local:
#include "internal_input_item.h"


namespace Haruhi {

namespace EventBackendPrivate {

class InternalInputDialog: public QDialog
{
	Q_OBJECT

  public:
	InternalInputDialog (QWidget* parent);

	virtual ~InternalInputDialog() { }

	InternalInputItem*
	item() const;

	void
	clear();

	void
	from (InternalInputItem*);

	void
	apply (InternalInputItem*) const;

  private slots:
	void
	update_widgets();

	void
	validate_and_save();

  private:
	InternalInputItem*	_item;
	QLineEdit*			_name;
	QPushButton*		_save_button;
	QCheckBox*			_note_checkbox;
	QWidget*			_note_params;
	QComboBox*			_note_channel;
	QCheckBox*			_controller_checkbox;
	QWidget*			_controller_params;
	QComboBox*			_controller_channel;
	QSpinBox*			_controller_number;
	QCheckBox*			_controller_invert;
	QCheckBox*			_pitchbend_checkbox;
	QWidget*			_pitchbend_params;
	QComboBox*			_pitchbend_channel;
	QCheckBox*			_channel_pressure_checkbox;
	QWidget*			_channel_pressure_params;
	QComboBox*			_channel_pressure_channel;
	QCheckBox*			_channel_pressure_invert;
	QCheckBox*			_key_pressure_checkbox;
	QWidget*			_key_pressure_params;
	QComboBox*			_key_pressure_channel;
	QCheckBox*			_key_pressure_invert;
};

} // namespace EventBackendPrivate

} // namespace Haruhi

#endif

