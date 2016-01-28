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

// Standard:
#include <cstddef>

// Qt:
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QKeyEvent>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>

// Local:
#include "controller_dialog.h"


namespace Haruhi {

namespace DevicesManager {

ControllerDialog::ControllerDialog (QWidget* parent):
	QDialog (parent)
{
	setWindowTitle ("Controller configuration");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	// Port name:

	auto name_label = new QLabel ("Controller name:", this);
	_name = std::make_unique<QLineEdit> (this);
	QObject::connect (_name.get(), SIGNAL (textChanged (const QString&)), this, SLOT (update_widgets()));

	// Filters:

	auto filters = new QGroupBox ("Accepted events", this);

	// Note filters:

	_note_checkbox = std::make_unique<QCheckBox> ("Note on/off", filters);
	QObject::connect (_note_checkbox.get(), SIGNAL (clicked()), this, SLOT (update_widgets()));

	_note_channel = create_channel_spinbox (filters);

	_note_velocity_checkbox = std::make_unique<QCheckBox> ("Note velocity", filters);
	QObject::connect (_note_velocity_checkbox.get(), SIGNAL (clicked()), this, SLOT (update_widgets()));

	_note_velocity_channel = create_channel_spinbox (filters);

	_note_pitch_checkbox = std::make_unique<QCheckBox> ("Note pitch", filters);
	QObject::connect (_note_pitch_checkbox.get(), SIGNAL (clicked()), this, SLOT (update_widgets()));

	_note_pitch_channel = create_channel_spinbox (filters);

	// Controller filters:

	_controller_checkbox = std::make_unique<QCheckBox> ("Controller", filters);
	QObject::connect (_controller_checkbox.get(), SIGNAL (clicked()), this, SLOT (update_widgets()));

	_controller_channel = create_channel_spinbox (filters);

	_controller_number = std::make_unique<QSpinBox> (filters);
	_controller_number->setRange (0, 127);
	_controller_number->setPrefix ("CC #");
	_controller_number->setToolTip ("MIDI controller number");

	_controller_invert = std::make_unique<QCheckBox> ("Invert", filters);
	_controller_invert->setToolTip ("Invert values");

	// Channel pressure filters:

	_channel_pressure_checkbox = std::make_unique<QCheckBox> ("Channel pressure", filters);
	QObject::connect (_channel_pressure_checkbox.get(), SIGNAL (clicked()), this, SLOT (update_widgets()));

	_channel_pressure_channel = create_channel_spinbox (filters);

	_channel_pressure_invert = std::make_unique<QCheckBox> ("Invert", filters);
	_channel_pressure_invert->setToolTip ("Invert values");

	// Key pressure filters:

	_key_pressure_checkbox = std::make_unique<QCheckBox> ("Key pressure", filters);
	QObject::connect (_key_pressure_checkbox.get(), SIGNAL (clicked()), this, SLOT (update_widgets()));

	_key_pressure_channel = create_channel_spinbox (filters);

	_key_pressure_invert = std::make_unique<QCheckBox> ("Invert", filters);
	_key_pressure_invert->setToolTip ("Invert values");

	// Pitchbend filters:

	_pitchbend_checkbox = std::make_unique<QCheckBox> ("Pitchbend", filters);
	QObject::connect (_pitchbend_checkbox.get(), SIGNAL (clicked()), this, SLOT (update_widgets()));

	_pitchbend_channel = create_channel_spinbox (filters);

	// Smoothing:

	_smoothing_label = std::make_unique<QLabel> ("Smoothing:", filters);

	_smoothing = std::make_unique<QSpinBox> (filters);
	_smoothing->setRange (0, 1000);
	_smoothing->setSingleStep (50);
	_smoothing->setSuffix (" ms");
	_smoothing->setSpecialValueText ("Off");

	// Buttons:

	_save_button = std::make_unique<QPushButton> (Resources::Icons16::ok(), "&Apply", this);
	_save_button->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_save_button.get(), SIGNAL (clicked()), this, SLOT (validate_and_save()));

	// Layout:

	auto layout = new QVBoxLayout (this);
	layout->setMargin (Config::dialog_margin());
	layout->setSpacing (Config::spacing());
	layout->setSizeConstraint (QLayout::SetNoConstraint);

	auto name_layout = new QHBoxLayout();
	name_layout->setSpacing (Config::spacing());
	name_layout->addWidget (name_label);
	name_layout->addWidget (_name.get());

	auto filters_layout = new QGridLayout (filters);
	filters_layout->addWidget (_note_checkbox.get(), 0, 0);
	filters_layout->addWidget (_note_channel.get(), 0, 2);
	filters_layout->addWidget (_note_velocity_checkbox.get(), 1, 0);
	filters_layout->addWidget (_note_velocity_channel.get(), 1, 2);
	filters_layout->addWidget (_note_pitch_checkbox.get(), 2, 0);
	filters_layout->addWidget (_note_pitch_channel.get(), 2, 2);
	filters_layout->addWidget (_controller_checkbox.get(), 3, 0);
	filters_layout->addWidget (_controller_number.get(), 3, 1);
	filters_layout->addWidget (_controller_channel.get(), 3, 2);
	filters_layout->addWidget (_controller_invert.get(), 3, 3);
	filters_layout->addWidget (_channel_pressure_checkbox.get(), 4, 0);
	filters_layout->addWidget (_channel_pressure_channel.get(), 4, 2);
	filters_layout->addWidget (_channel_pressure_invert.get(), 4, 3);
	filters_layout->addWidget (_key_pressure_checkbox.get(), 5, 0);
	filters_layout->addWidget (_key_pressure_channel.get(), 5, 2);
	filters_layout->addWidget (_key_pressure_invert.get(), 5, 3);
	filters_layout->addWidget (_pitchbend_checkbox.get(), 6, 0);
	filters_layout->addWidget (_pitchbend_channel.get(), 6, 2);
	filters_layout->addWidget (_smoothing_label.get(), 7, 0);
	filters_layout->addWidget (_smoothing.get(), 7, 2);

	auto buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::spacing());
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (_save_button.get());

	layout->addLayout (name_layout);
	layout->addWidget (filters);
	layout->addLayout (buttons_layout);

	update_widgets();

	adjustSize();
	setMinimumHeight (height());
	setMaximumHeight (height());
}


ControllerDialog::~ControllerDialog()
{ }


void
ControllerDialog::clear()
{
	_item = 0;
	setEnabled (false);
}


void
ControllerDialog::from (ControllerItem* item)
{
	auto controller = item->controller();
	setEnabled (true);
	_item = item;
	_name->setText (controller->name());
	_note_checkbox->setChecked (controller->note_filter);
	_note_channel->setValue (controller->note_channel);
	_note_velocity_checkbox->setChecked (controller->note_velocity_filter);
	_note_velocity_channel->setValue (controller->note_velocity_channel);
	_note_pitch_checkbox->setChecked (controller->note_pitch_filter);
	_note_pitch_channel->setValue (controller->note_pitch_channel);
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
	_smoothing->setValue (controller->smoothing.ms());
	_name->selectAll();
	_name->setFocus();
	update_widgets();
}


void
ControllerDialog::apply (ControllerItem* item) const
{
	auto controller = item->controller();
	controller->note_filter = _note_checkbox->isChecked();
	controller->note_channel = _note_channel->value();
	controller->note_velocity_filter = _note_velocity_checkbox->isChecked();
	controller->note_velocity_channel = _note_velocity_channel->value();
	controller->note_pitch_filter = _note_pitch_checkbox->isChecked();
	controller->note_pitch_channel = _note_pitch_channel->value();
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
	controller->smoothing = 1_ms * _smoothing->value();
	item->set_name (_name->text());
	emit item_configured (item);
}


void
ControllerDialog::keyPressEvent (QKeyEvent* event)
{
	if (event->key() == Qt::Key_Escape)
		return event->ignore();

	return QDialog::keyPressEvent (event);
}


void
ControllerDialog::update_widgets()
{
	bool note_checkbox = _note_checkbox->isChecked();
	_note_channel->setEnabled (note_checkbox);

	bool note_velocity_checkbox = _note_velocity_checkbox->isChecked();
	_note_velocity_channel->setEnabled (note_velocity_checkbox);

	bool note_pitch_checkbox = _note_pitch_checkbox->isChecked();
	_note_pitch_channel->setEnabled (note_pitch_checkbox);

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

	_save_button->setEnabled (!_name->text().isEmpty());
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


Unique<QSpinBox>
ControllerDialog::create_channel_spinbox (QWidget* parent)
{
	auto spinbox = std::make_unique<QSpinBox> (parent);
	spinbox->setRange (0, 16);
	spinbox->setPrefix ("Channel ");
	spinbox->setSpecialValueText ("All channels");
	return spinbox;
}

} // namespace DevicesManager

} // namespace Haruhi

