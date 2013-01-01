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
#include <set>

// Qt:
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QToolTip>
#include <QtGui/QMessageBox>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>
#include <QtGui/QTextDocument>
#include <QtGui/QApplication>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/exception.h>

// Local:
#include "preset_editor.h"
#include "package_item.h"
#include "category_item.h"
#include "preset_item.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

PresetEditor::PresetEditor (PresetsManager* presets_manager, QWidget* parent):
	QWidget (parent),
	_package_item (0),
	_category_item (0),
	_preset_item (0),
	_presets_manager (presets_manager)
{
	setSizePolicy (QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

	QGroupBox* grid = new QGroupBox (this);
	grid->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	_package = new QLineEdit (grid);
	_category = new QLineEdit (grid);
	_name = new QLineEdit (grid);
	_version = new QLineEdit (grid);
	_favorite = new QCheckBox ("Favorite preset", grid);

	connect (_package, SIGNAL (returnPressed()), SLOT (update_details()));
	connect (_category, SIGNAL (returnPressed()), SLOT (update_details()));
	connect (_name, SIGNAL (returnPressed()), SLOT (update_details()));
	connect (_version, SIGNAL (returnPressed()), SLOT (update_details()));

	QGridLayout* grid_layout = new QGridLayout (grid);
	grid_layout->addWidget (new QLabel ("Package:", grid), 0, 0);
	grid_layout->addWidget (_package, 0, 1);
	grid_layout->addWidget (new QLabel ("Category:", grid), 1, 0);
	grid_layout->addWidget (_category, 1, 1);
	grid_layout->addWidget (new QLabel ("Preset name:", grid), 2, 0);
	grid_layout->addWidget (_name, 2, 1);
	grid_layout->addWidget (new QLabel ("Version:", grid), 3, 0);
	grid_layout->addWidget (_version, 3, 1);
	grid_layout->addWidget (_favorite, 4, 0, 1, 2);

	_update_details_button = new QPushButton (Resources::Icons16::save(), "Save de&tails", this);
	_update_details_button->setAccel (Qt::CTRL + Qt::Key_T);
	QToolTip::add (_update_details_button, "Saves metadata without current patch");
	QObject::connect (_update_details_button, SIGNAL (clicked()), this, SLOT (update_details()));

	QHBoxLayout* hor_layout = new QHBoxLayout();
	hor_layout->setSpacing (Config::Spacing);
	hor_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	hor_layout->addWidget (_update_details_button);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (grid);
	layout->addLayout (hor_layout);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	update_widgets();
}


void
PresetEditor::clear()
{
	_package->setText ("");
	_name->setText ("");
	_category->clear();
	_category->setText ("");
	_version->setText ("");
	_favorite->setChecked (false);
}


void
PresetEditor::load_package (PackageItem* package_item)
{
	_package_item = package_item;
	_category_item = 0;
	_preset_item = 0;

	_package->setText (package_item->package()->name());
	_package->setEnabled (true);
	_category->setText ("");
	_category->setEnabled (false);
	_name->setText ("");
	_name->setEnabled (false);
	_version->setText ("");
	_version->setEnabled (false);
	_favorite->setChecked (false);
	_favorite->setEnabled (false);

	update_widgets();
}


void
PresetEditor::save_package (PackageItem* package_item)
{
	package_item->package()->set_name (_package->text());
	package_item->reload();
	_presets_manager->model()->changed();
}


void
PresetEditor::load_category (CategoryItem* category_item)
{
	_package_item = 0;
	_category_item = category_item;
	_preset_item = 0;

	_package->setText (category_item->package_item()->package()->name());
	_package->setEnabled (false);
	_category->setText (category_item->category()->name());
	_category->setEnabled (true);
	_name->setText ("");
	_name->setEnabled (false);
	_version->setText ("");
	_version->setEnabled (false);
	_favorite->setChecked (false);
	_favorite->setEnabled (false);
}


void
PresetEditor::save_category (CategoryItem* category_item)
{
	category_item->category()->set_name (_category->text());
	category_item->reload();
	_presets_manager->model()->changed();
}


void
PresetEditor::load_preset (PresetItem* preset_item)
{
	_package_item = 0;
	_category_item = 0;
	_preset_item = preset_item;

	_package->setText (preset_item->category_item()->package_item()->package()->name());
	_package->setEnabled (false);
	_category->setText (preset_item->category_item()->category()->name());
	_category->setEnabled (false);
	_name->setText (preset_item->preset()->name());
	_name->setEnabled (true);
	_version->setText (preset_item->preset()->version());
	_version->setEnabled (true);
	_favorite->setChecked (_presets_manager->favorited (preset_item->preset()->uuid()));
	_favorite->setEnabled (true);
}


void
PresetEditor::save_preset (PresetItem* preset_item)
{
	preset_item->preset()->set_name (_name->text());
	preset_item->preset()->set_version (_version->text());
	_presets_manager->set_favorited (preset_item->preset()->uuid(), _favorite->isChecked());
	preset_item->reload();
	_presets_manager->model()->changed();
}


void
PresetEditor::focus_package()
{
	_package->setFocus();
	_package->selectAll();
}


void
PresetEditor::focus_category()
{
	_category->setFocus();
	_category->selectAll();
}


void
PresetEditor::focus_name()
{
	_name->setFocus();
	_name->selectAll();
}


void
PresetEditor::update_details()
{
	QWidget* focused = QApplication::focusWidget();
	try {
		if (_package_item)
		{
			save_package (_package_item);
			_presets_manager->model()->save_state();
		}
		else if (_category_item)
		{
			save_category (_category_item);
			_presets_manager->model()->save_state();
		}
		else if (_preset_item)
		{
			save_preset (_preset_item);
			_presets_manager->model()->save_state();
		}
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Error", Qt::escape (e.what()));
	}
	if (focused)
		focused->setFocus();
}


void
PresetEditor::update_widgets()
{
	_update_details_button->setDefault (true);
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

