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
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <QtGui/QGridLayout>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>
#include <haruhi/widgets/styled_background.h>

// Local:
#include "device_dialog.h"


namespace Haruhi {

namespace DevicesManager {

DeviceDialog::DeviceDialog (QWidget* parent, Flags flags):
	QDialog (parent),
	_flags (flags)
{
	setWindowTitle ("Device configuration");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	QLabel* name_label = new QLabel ("Device name:", this);
	_name = new QLineEdit (this);
	_save_button = new QPushButton (Resources::Icons16::ok(), "&Apply", this);
	_auto_add_checkbox = new QCheckBox ("Auto add this device to new sessions", this);
	if ((flags & DisplayAutoAdd) == 0)
		_auto_add_checkbox->hide();

	QObject::connect (_name, SIGNAL (textChanged (const QString&)), this, SLOT (update_widgets()));
	QObject::connect (_save_button, SIGNAL (clicked()), this, SLOT (validate_and_save()));

	QGridLayout* grid_layout = new QGridLayout();
	grid_layout->setSpacing (Config::spacing());
	grid_layout->addWidget (name_label, 0, 0);
	grid_layout->addWidget (_name, 0, 1);
	grid_layout->addWidget (_auto_add_checkbox, 1, 1);

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::spacing());
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (_save_button);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::dialog_margin());
	layout->setSpacing (Config::spacing());
	layout->setSizeConstraint (QLayout::SetNoConstraint);
	layout->addLayout (grid_layout);
	layout->addLayout (buttons_layout);

	update_widgets();

	adjustSize();
	setMinimumHeight (height());
	setMaximumHeight (height());
}


void
DeviceDialog::clear()
{
	_item = 0;
	setEnabled (false);
}


void
DeviceDialog::from (DeviceItem* item)
{
	setEnabled (true);
	_item = item;
	_name->setText (item->name());
	_name->selectAll();
	_name->setFocus();
	_auto_add_checkbox->setChecked (item->device()->auto_add());
}


void
DeviceDialog::apply (DeviceItem* item) const
{
	item->set_name (_name->text());
	item->device()->set_auto_add (_auto_add_checkbox->isChecked());
	emit item_configured (item);
}


void
DeviceDialog::keyPressEvent (QKeyEvent* event)
{
	if (event->key() == Qt::Key_Escape)
		return event->ignore();

	return QDialog::keyPressEvent (event);
}


void
DeviceDialog::update_widgets()
{
	_save_button->setEnabled (!_name->text().isEmpty());
	_save_button->setDefault (true);
}


void
DeviceDialog::validate_and_save()
{
	if (_name->text().isEmpty())
		QMessageBox::warning (this, "Device name", "Enter name for the device.");
	else if (_item)
		apply (_item);
}

} // namespace DevicesManager

} // namespace Haruhi

