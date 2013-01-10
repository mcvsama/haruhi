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
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QLayout>
#include <QtGui/QLabel>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "input_dialog.h"


namespace Haruhi {

namespace AudioBackendImpl {

InputDialog::InputDialog (QWidget* parent, Backend* backend):
	PortDialog (parent, backend)
{
	setWindowTitle ("Input port configuration");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	QLabel* name_label = new QLabel ("Port name:", this);

	_name = new QLineEdit (this);
	QObject::connect (_name, SIGNAL (textChanged (const QString&)), this, SLOT (update_widgets()));

	_accept_button = new QPushButton ("&Ok", this);
	_accept_button->setDefault (true);
	QObject::connect (_accept_button, SIGNAL (clicked()), this, SLOT (validate_and_accept()));

	_reject_button = new QPushButton ("&Cancel", this);
	QObject::connect (_reject_button, SIGNAL (clicked()), this, SLOT (reject()));

	// Layouts:

	QHBoxLayout* name_layout = new QHBoxLayout();
	name_layout->setSpacing (Config::Spacing);
	name_layout->addWidget (name_label);
	name_layout->addWidget (_name);

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::Spacing);
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (_accept_button);
	buttons_layout->addWidget (_reject_button);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::DialogMargin);
	layout->setSpacing (Config::Spacing);
	layout->setSizeConstraint (QLayout::SetNoConstraint);
	layout->addLayout (name_layout);
	layout->addLayout (buttons_layout);

	update_widgets();

	adjustSize();
	setMinimumHeight (height());
	setMaximumHeight (height());
}


void
InputDialog::from (InputItem const* item)
{
	_name->setText (item->name());
}


void
InputDialog::apply (InputItem* item) const
{
	item->setText (0, _name->text());
	item->update_name();
}


QString
InputDialog::name() const
{
	return _name->text();
}


void
InputDialog::update_widgets()
{
	_accept_button->setEnabled (!_name->text().isEmpty());
}


void
InputDialog::validate_and_accept()
{
	if (_name->text().isEmpty())
		QMessageBox::warning (this, "Port name", "Enter name for the port.");
	else
		accept();
}

} // namespace AudioBackendImpl

} // namespace Haruhi

