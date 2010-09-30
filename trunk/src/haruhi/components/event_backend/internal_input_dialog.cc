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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <Qt3Support/Q3GroupBox>

// Haruhi:
#include <haruhi/config.h>

// Local:
#include "internal_input_dialog.h"


namespace Haruhi {

namespace EventBackendPrivate {

InternalInputDialog::InternalInputDialog (QWidget* parent):
	QDialog (parent)
{
	setCaption ("Controller configuration");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::dialog_margin, Config::spacing);
	layout->setResizeMode (QLayout::FreeResize);

		// Port name:

		QHBoxLayout* name_layout = new QHBoxLayout (layout, Config::spacing);

			QLabel* name_label = new QLabel ("Controller name:", this);

			_name = new QLineEdit (this);

		name_layout->addWidget (name_label);
		name_layout->addWidget (_name);

		// Filters:

		Q3GroupBox* filters = new Q3GroupBox (1, Qt::Horizontal, "MIDI filters", this);
		layout->addWidget (filters);

			// Note filters:

			_note_checkbox = new QCheckBox ("Note on/note off events", filters);
			QObject::connect (_note_checkbox, SIGNAL (clicked()), this, SLOT (update_widgets()));

			_note_params = new QWidget (filters);
			{
				QHBoxLayout* l = new QHBoxLayout (_note_params, 0, Config::spacing);

				QLabel* channel_label = new QLabel ("Channel:", _note_params);
				_note_channel = new QComboBox (_note_params);
				_note_channel->insertItem ("All");
				for (int i = 1; i <= 16; ++i)
					_note_channel->insertItem (QString ("%1").arg (i));

				l->addItem (new QSpacerItem (18, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
				l->addWidget (channel_label);
				l->addWidget (_note_channel);
				l->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
			}

			// Controller filters:

			_controller_checkbox = new QCheckBox ("Controller events", filters);
			QObject::connect (_controller_checkbox, SIGNAL (clicked()), this, SLOT (update_widgets()));

			_controller_params = new QWidget (filters);
			{
				QVBoxLayout* v1 = new QVBoxLayout (_controller_params, 0, Config::spacing);
				QHBoxLayout* h1 = new QHBoxLayout (v1, Config::spacing);
				QHBoxLayout* h2 = new QHBoxLayout (v1, Config::spacing);

				QLabel* channel_label = new QLabel ("Channel:", _controller_params);
				_controller_channel = new QComboBox (_controller_params);
				_controller_channel->insertItem ("All");
				for (int i = 1; i <= 16; ++i)
					_controller_channel->insertItem (QString ("%1").arg (i));
				QLabel* number_label = new QLabel ("Controller #", _controller_params);
				_controller_number = new QSpinBox (0, 127, 1, _controller_params);
				_controller_invert = new QCheckBox ("Invert", _controller_params);

				h1->addItem (new QSpacerItem (18, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
				h1->addWidget (channel_label);
				h1->addWidget (_controller_channel);
				h1->addWidget (number_label);
				h1->addWidget (_controller_number);
				h1->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
				h2->addItem (new QSpacerItem (18, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
				h2->addWidget (_controller_invert);
				h2->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
			}

			// Pitchbend filters:

			_pitchbend_checkbox = new QCheckBox ("Pitchbend events", filters);
			QObject::connect (_pitchbend_checkbox, SIGNAL (clicked()), this, SLOT (update_widgets()));

			_pitchbend_params = new QWidget (filters);
			{
				QHBoxLayout* l = new QHBoxLayout (_pitchbend_params, 0, Config::spacing);

				QLabel* channel_label = new QLabel ("Channel:", _pitchbend_params);
				_pitchbend_channel = new QComboBox (_pitchbend_params);
				_pitchbend_channel->insertItem ("All");
				for (int i = 1; i <= 16; ++i)
					_pitchbend_channel->insertItem (QString ("%1").arg (i));

				l->addItem (new QSpacerItem (18, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
				l->addWidget (channel_label);
				l->addWidget (_pitchbend_channel);
				l->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
			}

			// Channel pressure filters:

			_channel_pressure_checkbox = new QCheckBox ("Channel pressure events", filters);
			QObject::connect (_channel_pressure_checkbox, SIGNAL (clicked()), this, SLOT (update_widgets()));

			_channel_pressure_params = new QWidget (filters);
			{
				QVBoxLayout* v1 = new QVBoxLayout (_channel_pressure_params, 0, Config::spacing);
				QHBoxLayout* h1 = new QHBoxLayout (v1, Config::spacing);
				QHBoxLayout* h2 = new QHBoxLayout (v1, Config::spacing);

				QLabel* channel_label = new QLabel ("Channel:", _channel_pressure_params);
				_channel_pressure_channel = new QComboBox (_channel_pressure_params);
				_channel_pressure_channel->insertItem ("All");
				for (int i = 1; i <= 16; ++i)
					_channel_pressure_channel->insertItem (QString ("%1").arg (i));
				_channel_pressure_invert = new QCheckBox ("Invert", _channel_pressure_params);

				h1->addItem (new QSpacerItem (18, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
				h1->addWidget (channel_label);
				h1->addWidget (_channel_pressure_channel);
				h1->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
				h2->addItem (new QSpacerItem (18, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
				h2->addWidget (_channel_pressure_invert);
				h2->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
			}

			// Key pressure filters:

			_key_pressure_checkbox = new QCheckBox ("Key pressure events", filters);
			QObject::connect (_key_pressure_checkbox, SIGNAL (clicked()), this, SLOT (update_widgets()));

			_key_pressure_params = new QWidget (filters);
			{
				QVBoxLayout* v1 = new QVBoxLayout (_key_pressure_params, 0, Config::spacing);
				QHBoxLayout* h1 = new QHBoxLayout (v1, Config::spacing);
				QHBoxLayout* h2 = new QHBoxLayout (v1, Config::spacing);

				QLabel* key_label = new QLabel ("Channel:", _key_pressure_params);
				_key_pressure_channel = new QComboBox (_key_pressure_params);
				_key_pressure_channel->insertItem ("All");
				for (int i = 1; i <= 16; ++i)
					_key_pressure_channel->insertItem (QString ("%1").arg (i));
				_key_pressure_invert = new QCheckBox ("Invert", _key_pressure_params);

				h1->addItem (new QSpacerItem (18, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
				h1->addWidget (key_label);
				h1->addWidget (_key_pressure_channel);
				h1->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
				h2->addItem (new QSpacerItem (18, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
				h2->addWidget (_key_pressure_invert);
				h2->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
			}

		layout->addItem (new QSpacerItem (0, Config::spacing, QSizePolicy::Fixed, QSizePolicy::Fixed));

		// Buttons:

		QHBoxLayout* buttons_layout = new QHBoxLayout (layout, Config::spacing);

			_save_button = new QPushButton (Config::Icons16::ok(), "&Apply", this);
			_save_button->setDefault (true);

		buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
		buttons_layout->addWidget (_save_button);

	QObject::connect (_name, SIGNAL (textChanged (const QString&)), this, SLOT (update_widgets()));
	QObject::connect (_save_button, SIGNAL (clicked()), this, SLOT (validate_and_save()));

	update_widgets();

	adjustSize();
	setMinimumHeight (height());
	setMaximumHeight (height());
}


void
InternalInputDialog::clear()
{
	_item = 0;
	setEnabled (false);
}


void
InternalInputDialog::from (InternalInputItem* item)
{
	setEnabled (true);
	_item = item;
	_name->setText (item->name());
	_note_checkbox->setChecked (item->_note_filter);
	_note_channel->setCurrentItem (item->_note_channel);
	_controller_checkbox->setChecked (item->_controller_filter);
	_controller_channel->setCurrentItem (item->_controller_channel);
	_controller_number->setValue (item->_controller_number);
	_controller_invert->setChecked (item->_controller_invert);
	_pitchbend_checkbox->setChecked (item->_pitchbend_filter);
	_pitchbend_channel->setCurrentItem (item->_pitchbend_channel);
	_channel_pressure_checkbox->setChecked (item->_channel_pressure_filter);
	_channel_pressure_channel->setCurrentItem (item->_channel_pressure_channel);
	_channel_pressure_invert->setChecked (item->_channel_pressure_invert);
	_name->selectAll();
	_name->setFocus();
	update_widgets();
}


void
InternalInputDialog::apply (InternalInputItem* item) const
{
	item->setText (0, _name->text());
	item->update_name();
	item->_note_filter = _note_checkbox->isChecked();
	item->_note_channel = _note_channel->currentText().toInt();
	item->_controller_filter = _controller_checkbox->isChecked();
	item->_controller_channel = _controller_channel->currentText().toInt();
	item->_controller_number = _controller_number->value();
	item->_controller_invert = _controller_invert->isChecked();
	item->_pitchbend_filter = _pitchbend_checkbox->isChecked();
	item->_pitchbend_channel = _pitchbend_channel->currentText().toInt();
	item->_channel_pressure_filter = _channel_pressure_checkbox->isChecked();
	item->_channel_pressure_channel = _channel_pressure_channel->currentText().toInt();
	item->_channel_pressure_invert = _channel_pressure_invert->isChecked();
}


void
InternalInputDialog::update_widgets()
{
	_save_button->setEnabled (!_name->text().isEmpty());
	_note_params->setEnabled (_note_checkbox->isChecked());
	_controller_params->setEnabled (_controller_checkbox->isChecked());
	_pitchbend_params->setEnabled (_pitchbend_checkbox->isChecked());
	_channel_pressure_params->setEnabled (_channel_pressure_checkbox->isChecked());
}


void
InternalInputDialog::validate_and_save()
{
	if (_name->text().isEmpty())
		QMessageBox::warning (this, "Controller name", "Enter name for the controller.");
	else if (_item)
		apply (_item);
}

} // namespace EventBackendPrivate

} // namespace Haruhi

