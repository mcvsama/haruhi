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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QToolTip>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "controller_dialog.h"


namespace Haruhi {

namespace DevicesManager {

ControllerDialog::ControllerDialog (QWidget* parent):
	QDialog (parent)
{
	setCaption ("Controller configuration");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	// Port name:

	QLabel* name_label = new QLabel ("Controller name:", this);
	_name = new QLineEdit (this);
	QObject::connect (_name, SIGNAL (textChanged (const QString&)), this, SLOT (update_widgets()));

	// Filters:

	QGroupBox* filters = new QGroupBox ("Accepted events", this);

	// Note filters:

	_note_checkbox = new QCheckBox ("Note on/off", filters);
	QObject::connect (_note_checkbox, SIGNAL (clicked()), this, SLOT (update_widgets()));

	_note_channel = create_channel_spinbox (filters);

	// Controller filters:

	_controller_checkbox = new QCheckBox ("Controller", filters);
	QObject::connect (_controller_checkbox, SIGNAL (clicked()), this, SLOT (update_widgets()));

	_controller_channel = create_channel_spinbox (filters);

	_controller_number = new QSpinBox (0, 127, 1, filters);
	_controller_number->setPrefix ("CC #");
	QToolTip::add (_controller_number, "MIDI controller number");

	_controller_invert = new QCheckBox ("Invert", filters);
	QToolTip::add (_controller_invert, "Invert values");

	// Channel pressure filters:

	_channel_pressure_checkbox = new QCheckBox ("Channel pressure", filters);
	QObject::connect (_channel_pressure_checkbox, SIGNAL (clicked()), this, SLOT (update_widgets()));

	_channel_pressure_channel = create_channel_spinbox (filters);

	_channel_pressure_invert = new QCheckBox ("Invert", filters);
	QToolTip::add (_channel_pressure_invert, "Invert values");

	// Key pressure filters:

	_key_pressure_checkbox = new QCheckBox ("Key pressure", filters);
	QObject::connect (_key_pressure_checkbox, SIGNAL (clicked()), this, SLOT (update_widgets()));

	_key_pressure_channel = create_channel_spinbox (filters);

	_key_pressure_invert = new QCheckBox ("Invert", filters);
	QToolTip::add (_key_pressure_invert, "Invert values");

	// Pitchbend filters:

	_pitchbend_checkbox = new QCheckBox ("Pitchbend", filters);
	QObject::connect (_pitchbend_checkbox, SIGNAL (clicked()), this, SLOT (update_widgets()));

	_pitchbend_channel = create_channel_spinbox (filters);

	// Smoothing:

	_smoothing_label = new QLabel ("Smoothing:", filters);

	_smoothing = new QSpinBox (0, 1000, 50, filters);
	_smoothing->setSuffix (" ms");
	_smoothing->setSpecialValueText ("Off");

	// Buttons:

	_save_button = new QPushButton (Resources::Icons16::ok(), "&Apply", this);
	QObject::connect (_save_button, SIGNAL (clicked()), this, SLOT (validate_and_save()));

	// Layout:

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::DialogMargin);
	layout->setSpacing (Config::Spacing);
	layout->setResizeMode (QLayout::FreeResize);

	QHBoxLayout* name_layout = new QHBoxLayout();
	name_layout->setSpacing (Config::Spacing);
	name_layout->addWidget (name_label);
	name_layout->addWidget (_name);

	QGridLayout* filters_layout = new QGridLayout (filters);
	filters_layout->addWidget (_note_checkbox, 0, 0);
	filters_layout->addWidget (_note_channel, 0, 2);
	filters_layout->addWidget (_controller_checkbox, 1, 0);
	filters_layout->addWidget (_controller_number, 1, 1);
	filters_layout->addWidget (_controller_channel, 1, 2);
	filters_layout->addWidget (_controller_invert, 1, 3);
	filters_layout->addWidget (_channel_pressure_checkbox, 2, 0);
	filters_layout->addWidget (_channel_pressure_channel, 2, 2);
	filters_layout->addWidget (_channel_pressure_invert, 2, 3);
	filters_layout->addWidget (_key_pressure_checkbox, 3, 0);
	filters_layout->addWidget (_key_pressure_channel, 3, 2);
	filters_layout->addWidget (_key_pressure_invert, 3, 3);
	filters_layout->addWidget (_pitchbend_checkbox, 4, 0);
	filters_layout->addWidget (_pitchbend_channel, 4, 2);
	filters_layout->addWidget (_smoothing_label, 5, 0);
	filters_layout->addWidget (_smoothing, 5, 2);

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::Spacing);
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (_save_button);

	layout->addLayout (name_layout);
	layout->addWidget (filters);
	layout->addLayout (buttons_layout);
	layout->addItem (new QSpacerItem (0, Config::Spacing, QSizePolicy::Fixed, QSizePolicy::Fixed));

	update_widgets();

	adjustSize();
	setMinimumHeight (height());
	setMaximumHeight (height());
}


void
ControllerDialog::clear()
{
	_item = 0;
	setEnabled (false);
}


void
ControllerDialog::from (ControllerItem* item)
{
	Controller* controller = item->controller();
	setEnabled (true);
	_item = item;
	_name->setText (controller->name());
	_note_checkbox->setChecked (controller->note_filter);
	_note_channel->setValue (controller->note_channel);
	_controller_checkbox->setChecked (controller->controller_filter);
	_controller_channel->setValue (controller->controller_channel);
	_controller_number->setValue (controller->controller_number);
	_controller_invert->setChecked (controller->controller_invert);
	_pitchbend_checkbox->setChecked (controller->pitchbend_filter);
	_pitchbend_channel->setValue (controller->pitchbend_channel);
	_channel_pressure_checkbox->setChecked (controller->channel_pressure_filter);
	_channel_pressure_channel->setValue (controller->channel_pressure_channel);
	_channel_pressure_invert->setChecked (controller->channel_pressure_invert);
	_key_pressure_checkbox->setChecked (controller->key_pressure_filter);
	_key_pressure_channel->setValue (controller->key_pressure_channel);
	_key_pressure_invert->setChecked (controller->key_pressure_invert);
	_smoothing->setValue (controller->smoothing);
	_name->selectAll();
	_name->setFocus();
	update_widgets();
}


void
ControllerDialog::apply (ControllerItem* item) const
{
	Controller* controller = item->controller();
	controller->note_filter = _note_checkbox->isChecked();
	controller->note_channel = _note_channel->value();
	controller->controller_filter = _controller_checkbox->isChecked();
	controller->controller_channel = _controller_channel->value();
	controller->controller_number = _controller_number->value();
	controller->controller_invert = _controller_invert->isChecked();
	controller->pitchbend_filter = _pitchbend_checkbox->isChecked();
	controller->pitchbend_channel = _pitchbend_channel->value();
	controller->channel_pressure_filter = _channel_pressure_checkbox->isChecked();
	controller->channel_pressure_channel = _channel_pressure_channel->value();
	controller->channel_pressure_invert = _channel_pressure_invert->isChecked();
	controller->key_pressure_filter = _key_pressure_checkbox->isChecked();
	controller->key_pressure_channel = _key_pressure_channel->value();
	controller->key_pressure_invert = _key_pressure_invert->isChecked();
	controller->smoothing = _smoothing->value();
	item->set_name (_name->text());
	emit item_configured (item);
}


void
ControllerDialog::update_widgets()
{
	bool note_checkbox = _note_checkbox->isChecked();
	_save_button->setEnabled (!_name->text().isEmpty());
	_note_channel->setEnabled (note_checkbox);

	bool controller_checkbox = _controller_checkbox->isChecked();
	_controller_channel->setEnabled (controller_checkbox);
	_controller_number->setEnabled (controller_checkbox);
	_controller_invert->setEnabled (controller_checkbox);

	bool pitchbend_checkbox = _pitchbend_checkbox->isChecked();
	_pitchbend_channel->setEnabled (pitchbend_checkbox);

	bool channel_pressure_checkbox = _channel_pressure_checkbox->isChecked();
	_channel_pressure_channel->setEnabled (channel_pressure_checkbox);
	_channel_pressure_invert->setEnabled (channel_pressure_checkbox);

	bool key_pressure_checkbox = _key_pressure_checkbox->isChecked();
	_key_pressure_channel->setEnabled (key_pressure_checkbox);
	_key_pressure_invert->setEnabled (key_pressure_checkbox);

	bool smoothing = _controller_checkbox->isChecked() || _pitchbend_checkbox->isChecked() ||
					 _channel_pressure_checkbox->isChecked() || _key_pressure_checkbox->isChecked();
	_smoothing_label->setEnabled (smoothing);
	_smoothing->setEnabled (smoothing);

	_save_button->setDefault (true);
}


void
ControllerDialog::validate_and_save()
{
	if (_name->text().isEmpty())
		QMessageBox::warning (this, "Controller name", "Enter name for the controller.");
	else if (_item)
		apply (_item);
}


QSpinBox*
ControllerDialog::create_channel_spinbox (QWidget* parent)
{
	QSpinBox* spinbox = new QSpinBox (0, 16, 1, parent);
	spinbox->setPrefix ("Channel ");
	spinbox->setSpecialValueText ("All channels");
	return spinbox;
}

} // namespace DevicesManager

} // namespace Haruhi

