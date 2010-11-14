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
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <Qt3Support/Q3ButtonGroup>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "create_port_dialog.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

CreatePortDialog::Graph CreatePortDialog::_last_selected_port_type = CreatePortDialog::Audio;


CreatePortDialog::CreatePortDialog (QWidget* parent):
	QDialog (parent)
{
	setCaption ("Create port");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	_layout = new QVBoxLayout (this, Config::DialogMargin, Config::Spacing);
	_layout->setResizeMode (QLayout::FreeResize);

		Q3ButtonGroup* group1 = new Q3ButtonGroup (2, Qt::Vertical, "Port type", this);
		group1->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

			_button_audio = new QRadioButton ("Audio", group1);
			_button_event = new QRadioButton ("Event", group1);

			switch (_last_selected_port_type)
			{
				case CreatePortDialog::Audio:	_button_audio->setChecked (true); break;
				case CreatePortDialog::Event:	_button_event->setChecked (true); break;
			}

		Q3GroupBox* group2 = new Q3GroupBox (1, Qt::Vertical, "Port name", this);
		group2->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

			_port_name = new QLineEdit (group2);
			_port_name->setFocus();

		QWidget* buttons = new QWidget (this);
		buttons->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

		QHBoxLayout* buttons_layout = new QHBoxLayout (buttons, 0, Config::Spacing);

			_accept_button = new QPushButton ("&Ok", buttons);
			_accept_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Maximum);
			_accept_button->setDefault (true);

			_reject_button = new QPushButton ("&Cancel", buttons);
			_reject_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Maximum);

		buttons_layout->addItem (new QSpacerItem (0, 0));
		buttons_layout->addWidget (_accept_button);
		buttons_layout->addWidget (_reject_button);

	_layout->addWidget (group1);
	_layout->addWidget (group2);
	_layout->addWidget (buttons);

	QObject::connect (_port_name, SIGNAL (textChanged (const QString&)), this, SLOT (state_changed()));
	QObject::connect (_button_audio, SIGNAL (clicked()), this, SLOT (state_changed()));
	QObject::connect (_button_event, SIGNAL (clicked()), this, SLOT (state_changed()));
	QObject::connect (_accept_button, SIGNAL (clicked()), this, SLOT (validate_and_accept()));
	QObject::connect (_reject_button, SIGNAL (clicked()), this, SLOT (reject()));

	state_changed();

	adjustSize();
	setMinimumHeight (height());
	setMaximumHeight (height());
}


CreatePortDialog::Graph
CreatePortDialog::port_type() const
{
	return _button_audio->isChecked()? CreatePortDialog::Audio : CreatePortDialog::Event;
}


QString
CreatePortDialog::port_name() const
{
	return _port_name->text();
}


void
CreatePortDialog::state_changed()
{
	_last_selected_port_type = _button_audio->isChecked()? CreatePortDialog::Audio : CreatePortDialog::Event;
	_accept_button->setEnabled (!_port_name->text().isEmpty());
}


void
CreatePortDialog::validate_and_accept()
{
	if (_port_name->text().isEmpty())
		QMessageBox::warning (this, "Port name", "Enter name for port");
	else
		accept();
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

