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
#include <QMessageBox>
#include <QPushButton>
#include <QLayout>
#include <QLabel>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>

// Local:
#include "output_dialog.h"


namespace Haruhi {

namespace AudioBackendImpl {

OutputDialog::OutputDialog (QWidget* parent, Backend* backend):
	PortDialog (parent, backend)
{
	setWindowTitle ("Output port configuration");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	auto name_label = new QLabel ("Port name:", this);

	_name = std::make_unique<QLineEdit> (this);
	QObject::connect (_name.get(), SIGNAL (textChanged (const QString&)), this, SLOT (update_widgets()));

	_accept_button = std::make_unique<QPushButton> ("&Ok", this);
	_accept_button->setDefault (true);
	QObject::connect (_accept_button.get(), SIGNAL (clicked()), this, SLOT (validate_and_accept()));

	_reject_button = std::make_unique<QPushButton> ("&Cancel", this);
	QObject::connect (_reject_button.get(), SIGNAL (clicked()), this, SLOT (reject()));

	// Layout:

	auto name_layout = new QHBoxLayout();
	name_layout->setSpacing (Config::spacing());
	name_layout->addWidget (name_label);
	name_layout->addWidget (_name.get());

	auto buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::spacing());
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (_accept_button.get());
	buttons_layout->addWidget (_reject_button.get());

	auto layout = new QVBoxLayout (this);
	layout->setMargin (Config::dialog_margin());
	layout->setSpacing (Config::spacing());
	layout->setSizeConstraint (QLayout::SetNoConstraint);
	layout->addLayout (name_layout);
	layout->addLayout (buttons_layout);

	update_widgets();

	adjustSize();
	setMinimumHeight (height());
	setMaximumHeight (height());
}


void
OutputDialog::from (OutputItem const* item)
{
	_name->setText (item->name());
}


void
OutputDialog::apply (OutputItem* item) const
{
	item->setText (0, _name->text());
	item->update_name();
}


QString
OutputDialog::name() const
{
	return _name->text();
}


void
OutputDialog::update_widgets()
{
	_accept_button->setEnabled (!_name->text().isEmpty());
}


void
OutputDialog::validate_and_accept()
{
	if (_name->text().isEmpty())
		QMessageBox::warning (this, "Port name", "Enter name for the port.");
	else
		accept();
}

} // namespace AudioBackendImpl

} // namespace Haruhi

