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

// Haruhi:
#include <haruhi/config.h>

// Local:
#include "external_input_dialog.h"


namespace Haruhi {

namespace EventBackendPrivate {

ExternalInputDialog::ExternalInputDialog (QWidget* parent):
	QWidget (parent)
{
	setCaption ("External input port configuration");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::dialog_margin, Config::spacing);
	layout->setResizeMode (QLayout::FreeResize);

		QHBoxLayout* name_layout = new QHBoxLayout (layout, Config::spacing);

			QLabel* name_label = new QLabel ("External port name:", this);

			_name = new QLineEdit (this);

		name_layout->addWidget (name_label);
		name_layout->addWidget (_name);

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
ExternalInputDialog::clear()
{
	_item = 0;
	setEnabled (false);
}


void
ExternalInputDialog::from (ExternalInputItem* item)
{
	setEnabled (true);
	_item = item;
	_name->setText (item->name());
	_name->selectAll();
	_name->setFocus();
}


void
ExternalInputDialog::apply (ExternalInputItem* item) const
{
	item->setText (0, _name->text());
	item->update_name();
}


void
ExternalInputDialog::update_widgets()
{
	_save_button->setEnabled (!_name->text().isEmpty());
}


void
ExternalInputDialog::validate_and_save()
{
	if (_name->text().isEmpty())
		QMessageBox::warning (this, "External port name", "Enter name for the external port.");
	else if (_item)
		apply (_item);
}

} // namespace EventBackendPrivate

} // namespace Haruhi

