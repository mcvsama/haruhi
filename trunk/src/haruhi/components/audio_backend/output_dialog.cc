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
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QLayout>
#include <QtGui/QLabel>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "output_dialog.h"


namespace Haruhi {

namespace AudioBackend {

OutputDialog::OutputDialog (QWidget* parent, Backend* backend):
	PortDialog (parent, backend)
{
	setCaption ("Output port configuration");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::DialogMargin, Config::Spacing);
	layout->setResizeMode (QLayout::FreeResize);

		QHBoxLayout* name_layout = new QHBoxLayout (layout, Config::Spacing);

			QLabel* name_label = new QLabel ("Port name:", this);

			_name = new QLineEdit (this);

		name_layout->addWidget (name_label);
		name_layout->addWidget (_name);

		QHBoxLayout* buttons_layout = new QHBoxLayout (layout, Config::Spacing);

			_accept_button = new QPushButton ("&Ok", this);
			_accept_button->setDefault (true);

			_reject_button = new QPushButton ("&Cancel", this);

		buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
		buttons_layout->addWidget (_accept_button);
		buttons_layout->addWidget (_reject_button);

	QObject::connect (_name, SIGNAL (textChanged (const QString&)), this, SLOT (update_widgets()));
	QObject::connect (_accept_button, SIGNAL (clicked()), this, SLOT (validate_and_accept()));
	QObject::connect (_reject_button, SIGNAL (clicked()), this, SLOT (reject()));

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

} // namespace AudioBackend

} // namespace Haruhi

