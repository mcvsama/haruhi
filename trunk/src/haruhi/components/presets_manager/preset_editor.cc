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
#include <set>

// Qt:
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QToolTip>
#include <QtGui/QMessageBox>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

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
	QGridLayout* grid_layout = new QGridLayout (grid);

	grid_layout->addWidget (new QLabel ("Package:", grid), 0, 0);
	grid_layout->addWidget (_package = new QLineEdit (grid), 0, 1);

	grid_layout->addWidget (new QLabel ("Category:", grid), 1, 0);
	grid_layout->addWidget (_category = new QLineEdit (grid), 1, 1);

	grid_layout->addWidget (new QLabel ("Preset name:", grid), 2, 0);
	grid_layout->addWidget (_name = new QLineEdit (grid), 2, 1);

	grid_layout->addWidget (new QLabel ("Version:", grid), 3, 0);
	grid_layout->addWidget (_version = new QLineEdit (grid), 3, 1);

	grid_layout->addWidget (_favorite = new QCheckBox ("Favorite preset", grid), 4, 0, 1, 2);

	_update_details_button = new QPushButton (Resources::Icons16::save(), "Update de&tails", this);
	_update_details_button->setAccel (Qt::CTRL + Qt::Key_T);
	QToolTip::add (_update_details_button, "Saves metadata without current patch");
	QObject::connect (_update_details_button, SIGNAL (clicked()), this, SLOT (update_details()));

	QVBoxLayout* v1 = new QVBoxLayout (this, 0, Config::Spacing);
	v1->addWidget (grid);
	QHBoxLayout* h1 = new QHBoxLayout (v1, Config::Spacing);
	h1->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	h1->addWidget (_update_details_button);
	v1->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
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

	_package->setText (package_item->meta().name);
	_package->setEnabled (true);
	_category->setText ("");
	_category->setEnabled (false);
	_name->setText ("");
	_name->setEnabled (false);
	_version->setText ("");
	_version->setEnabled (false);
	_favorite->setChecked (false);
	_favorite->setEnabled (false);
}


void
PresetEditor::save_package (PackageItem* package_item)
{
	package_item->meta().name = _package->text();
	package_item->reload();
}


void
PresetEditor::load_category (CategoryItem* category_item)
{
	_package_item = 0;
	_category_item = category_item;
	_preset_item = 0;

	_package->setText (category_item->package_item()->meta().name);
	_package->setEnabled (false);
	_category->setText (category_item->name());
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
	category_item->set_name (_category->text());
	category_item->reload();
}


void
PresetEditor::load_preset (PresetItem* preset_item)
{
	_package_item = 0;
	_category_item = 0;
	_preset_item = preset_item;

	PresetItem::Meta const& m = preset_item->meta();
	_package->setText (preset_item->category_item()->package_item()->meta().name);
	_package->setEnabled (false);
	_category->setText (preset_item->category_item()->name());
	_category->setEnabled (false);
	_name->setText (m.name);
	_name->setEnabled (true);
	_version->setText (m.version);
	_version->setEnabled (true);
	_favorite->setChecked (_presets_manager->favorited (_preset_item->uuid()));
	_favorite->setEnabled (true);
}


void
PresetEditor::save_preset (PresetItem* preset_item)
{
	PresetItem::Meta& m = preset_item->meta();
	m.name = _name->text();
	m.version = _version->text();
	_presets_manager->set_favorited (preset_item->uuid(), _favorite->isChecked());
	preset_item->reload();
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
	try {
		if (_package_item)
		{
			save_package (_package_item);
			_package_item->save_file();
		}
		else if (_category_item)
		{
			save_category (_category_item);
			_category_item->package_item()->save_file();
		}
		else if (_preset_item)
		{
			save_preset (_preset_item);
			_preset_item->category_item()->package_item()->save_file();
		}
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Error", e.what());
	}
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

