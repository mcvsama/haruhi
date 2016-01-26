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
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <set>

// System:
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

// Qt:
#include <QLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QTextDocument>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/graph/unit.h>
#include <haruhi/utility/exception.h>
#include <haruhi/utility/filesystem.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/memory.h>

// Local:
#include "package.h"
#include "category.h"
#include "preset.h"
#include "preset_editor.h"
#include "package_item.h"
#include "category_item.h"
#include "preset_item.h"
#include "presets_tree.h"
#include "presets_manager.h"


namespace Haruhi {

namespace Private = PresetsManagerPrivate;

PresetsManager::PresetsManager (Unit* unit, QWidget* parent):
	QWidget (parent),
	_unit (unit),
	_saveable_unit (dynamic_cast<SaveableState*> (unit))
{
	_has_presets_settings = Haruhi::haruhi()->has_presets_settings();

	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_create_menu = new QMenu (this);
	_create_package_action = _create_menu->addAction (Resources::Icons16::presets_package(), "Package", this, SLOT (create_package()));
	_create_category_action = _create_menu->addAction (Resources::Icons16::presets_category(), "Category", this, SLOT (create_category()));
	_create_preset_action = _create_menu->addAction (Resources::Icons16::preset(), "Preset", this, SLOT (create_preset()));

	_tree = new Private::PresetsTree (this, this);
	_tree->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	QObject::connect (_tree, SIGNAL (itemSelectionChanged()), this, SLOT (update_widgets()));
	QObject::connect (_tree, SIGNAL (itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT (load_preset (QTreeWidgetItem*)));

	_editor = new Private::PresetEditor (this, this);

	_only_favs_button = new QPushButton (Resources::Icons16::favorite(), "Show favorites only");
	_only_favs_button->setIconSize (Resources::Icons16::haruhi().size());
	_only_favs_button->setCheckable (true);
	QObject::connect (_only_favs_button, SIGNAL (toggled (bool)), this, SLOT (show_favorites()));

	_load_button = new QPushButton (Resources::Icons16::load(), "Load", this);
	_load_button->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_load_button, SIGNAL (clicked()), this, SLOT (load_preset()));

	_save_button = new QPushButton (Resources::Icons16::save(), "Save patch", this);
	_save_button->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_save_button, SIGNAL (clicked()), this, SLOT (save_preset()));

	_create_button = new QPushButton (Resources::Icons16::new_(), "Create", this);
	_create_button->setIconSize (Resources::Icons16::haruhi().size());
	_create_button->setMenu (_create_menu);

	_destroy_button = new QPushButton (Resources::Icons16::remove(), "Destroy…", this);
	_destroy_button->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_destroy_button, SIGNAL (clicked()), this, SLOT (destroy()));

	// Layouts:

	QVBoxLayout* v1 = new QVBoxLayout();
	v1->setSpacing (Config::spacing());
	v1->addWidget (_only_favs_button);
	v1->addWidget (_tree);

	QHBoxLayout* h2 = new QHBoxLayout();
	h2->setSpacing (Config::spacing());
	h2->addWidget (_load_button);
	h2->addWidget (_save_button);
	h2->addWidget (_create_button);
	h2->addWidget (_destroy_button);

	QVBoxLayout* v2 = new QVBoxLayout();
	v2->setMargin (0);
	v2->setSpacing (Config::spacing());
	v2->setSizeConstraint (QLayout::SetFixedSize);
	v2->addLayout (h2);
	v2->addWidget (_editor);

	QHBoxLayout* h1 = new QHBoxLayout (this);
	h1->setMargin (0);
	h1->setSpacing (Config::spacing());
	h1->addLayout (v1);
	h1->addLayout (v2);

	QString all_presets_dir = Settings::data_home() + "/presets";
	mkpath (all_presets_dir.toStdString(), 0700);

	std::string dir = sanitize_urn (_unit->urn());
	try {
		_model = Private::Model::get (all_presets_dir + "/" + QString::fromStdString (dir), QString::fromStdString (_unit->urn()));
		_model->on_change.connect (this, &PresetsManager::read);
		read();
	}
	catch (Exception const& e)
	{
		_model = 0;
		QString message =
			"Another instance of Haruhi has locked the presets directory.\n"
			"Presets manager will be disabled for this session.\n\nError message: %1\nDetail: %2";
		QMessageBox::information (0, "Presets disabled", message.arg (e.what()).arg (e.details()));
		setEnabled (false);
	}

	update_widgets();
}


PresetsManager::~PresetsManager()
{
	Private::Model::release (_model);
}


bool
PresetsManager::favorited (QString const& preset_uuid) const
{
	return _has_presets_settings->favorited (_unit->urn(), preset_uuid.toStdString());
}


void
PresetsManager::set_favorited (QString const& preset_uuid, bool set)
{
	_has_presets_settings->set_favorited (_unit->urn(), preset_uuid.toStdString(), set);
	_has_presets_settings->save();
}


void
PresetsManager::load_preset()
{
	if (_saveable_unit)
	{
		Private::PresetItem* preset_item = _tree->current_preset_item();
		if (preset_item)
		{
			_saveable_unit->load_state (preset_item->preset()->patch());
			emit preset_selected (preset_item->preset()->uuid(), preset_item->preset()->name());
		}
	}
}


void
PresetsManager::load_preset (QTreeWidgetItem* item)
{
	_tree->clearSelection();
	item->setSelected (true);
	load_preset();
}


void
PresetsManager::save_preset()
{
	Private::PresetItem* preset_item = _tree->current_preset_item();
	if (preset_item)
	{
		save_preset (preset_item, true);
		emit preset_selected (preset_item->preset()->uuid(), preset_item->preset()->name());
	}
}


void
PresetsManager::create_package()
{
	Private::Package* package = _model->create_package();
	model()->save_state();
	Private::PackageItem* package_item = create_package_item (package);
	// changed() should be called after adding the item, so read() won't add additional second item:
	_model->save_state();
	_model->changed();
	_tree->clearSelection();
	package_item->setSelected (true);
	_editor->focus_package();
}


void
PresetsManager::create_category()
{
	Private::PackageItem* package_item = _tree->current_package_item();
	if (!package_item)
	{
		Private::CategoryItem* category_item = _tree->current_category_item();
		if (category_item)
			package_item = category_item->package_item();
		else
		{
			Private::PresetItem* preset_item = _tree->current_preset_item();
			if (preset_item)
				package_item = preset_item->category_item()->package_item();
		}
	}

	if (package_item)
	{
		Private::Category* category = package_item->package()->create_category();
		Private::CategoryItem* category_item = new Private::CategoryItem (package_item, category);
		// changed() should be called after adding the item, so read() won't add additional second item:
		_model->save_state();
		_model->changed();
		_tree->clearSelection();
		category_item->setSelected (true);
		_editor->focus_category();
		package_item->setExpanded (true);
	}
}


void
PresetsManager::create_preset()
{
	Private::CategoryItem* category_item = _tree->current_category_item();
	if (!category_item)
	{
		Private::PresetItem* preset_item = _tree->current_preset_item();
		if (preset_item)
			category_item = preset_item->category_item();
	}

	if (category_item)
	{
		Private::Preset* preset = category_item->category()->create_preset();
		Private::PresetItem* preset_item = new Private::PresetItem (category_item, preset);
		_editor->load_preset (preset_item);
		save_preset (preset_item, true);
		// changed() should be called after adding the item, so read() won't add additional second item:
		_model->save_state();
		_model->changed();
		_tree->clearSelection();
		preset_item->setSelected (true);
		_editor->focus_name();
		category_item->setExpanded (true);
		category_item->package_item()->setExpanded (true);
	}
}


void
PresetsManager::destroy()
{
	Private::PackageItem* package_item = _tree->current_package_item();
	if (package_item)
	{
		if (QMessageBox::question (this, "Delete package", "Really delete package " + package_item->package()->name().toHtmlEscaped() + "?",
								   QMessageBox::Yes | QMessageBox::Default, QMessageBox::Cancel | QMessageBox::Escape) == QMessageBox::Yes)
		{
			Private::Package* package = package_item->package();
			remove_package_item (package_item);
			_model->remove_package (package);
			_model->changed();
		}
	}

	Private::CategoryItem* category_item = _tree->current_category_item();
	if (category_item)
	{
		if (QMessageBox::question (this, "Delete category", "Really delete category " + category_item->category()->name().toHtmlEscaped() + "?",
								   QMessageBox::Yes | QMessageBox::Default, QMessageBox::Cancel | QMessageBox::Escape) == QMessageBox::Yes)
		{
			try {
				Private::Category* category = category_item->category();
				Private::PackageItem* package_item = category_item->package_item();
				package_item->removeChild (category_item);
				delete category_item;
				package_item->package()->remove_category (category);
				_model->save_state();
				_model->changed();
			}
			catch (Exception const& e)
			{
				QMessageBox::warning (this, "Error", QString (e.what()).toHtmlEscaped());
			}
		}
	}

	Private::PresetItem* preset_item = _tree->current_preset_item();
	if (preset_item)
	{
		if (QMessageBox::question (this, "Delete preset", "Really delete preset " + preset_item->preset()->name().toHtmlEscaped() + "?",
								   QMessageBox::Yes | QMessageBox::Default, QMessageBox::Cancel | QMessageBox::Escape) == QMessageBox::Yes)
		{
			try {
				Private::Preset* preset = preset_item->preset();
				Private::CategoryItem* category_item = preset_item->category_item();
				category_item->removeChild (preset_item);
				delete preset_item;
				category_item->category()->remove_preset (preset);
				_model->save_state();
				_model->changed();
			}
			catch (Exception const& e)
			{
				QMessageBox::warning (this, "Error", QString (e.what()).toHtmlEscaped());
			}
		}
	}
}


void
PresetsManager::update_widgets()
{
	_load_button->setEnabled (_saveable_unit && _tree->current_preset_item());
	_save_button->setEnabled (_saveable_unit && _tree->current_preset_item());
	_create_button->setEnabled (_saveable_unit);
	_destroy_button->setEnabled (_saveable_unit);

	Private::PackageItem* package_item = _tree->current_package_item();
	Private::CategoryItem* category_item = _tree->current_category_item();
	Private::PresetItem* preset_item = _tree->current_preset_item();

	_editor->setEnabled (true);

	if (package_item)
	{
		_editor->load_package (package_item);
		_create_package_action->setEnabled (true);
		_create_category_action->setEnabled (true);
		_create_preset_action->setEnabled (false);
	}
	else if (category_item)
	{
		_editor->load_category (category_item);
		_create_package_action->setEnabled (true);
		_create_category_action->setEnabled (true);
		_create_preset_action->setEnabled (true);
	}
	else if (preset_item)
	{
		_editor->load_preset (preset_item);
		_create_package_action->setEnabled (true);
		_create_category_action->setEnabled (true);
		_create_preset_action->setEnabled (true);
	}
	else
	{
		_editor->setEnabled (false);
		_editor->clear();
		_create_package_action->setEnabled (true);
		_create_category_action->setEnabled (false);
		_create_preset_action->setEnabled (false);
	}
}


void
PresetsManager::show_favorites()
{
	bool only_favs = _only_favs_button->isChecked();
	// Iterate through items and show/hide them according to _only_favs_button
	// state and preset marked as favorite:
	for (QTreeWidgetItemIterator item (_tree); *item; ++item)
	{
		Private::PresetItem* preset_item = dynamic_cast<Private::PresetItem*> (*item);
		// If PresetItem, show/hide it.
		if (preset_item)
		{
			preset_item->setHidden (only_favs && !favorited (preset_item->preset()->uuid()));
			// If shown, ensure that parent items are shown, too.
			if (!preset_item->isHidden())
			{
				preset_item->parent()->setHidden (false);
				preset_item->parent()->parent()->setHidden (false);
			}
			// If item was hidden and selected, clear its selection:
			else
				preset_item->setSelected (false);
		}
		// By default hide PackageItem and CategoryItem. If any child PresetItem is shown,
		// it will also show parent items (this relies on fact that QTreeWidgetItemIterator goes
		// first through parent items, then through children).
		else
			(*item)->setHidden (only_favs);
	}
}


void
PresetsManager::read()
{
	typedef std::set<Private::Package*> PackagesSet;

	// Read packages:
	PackagesSet m_packages; // Model packages
	PackagesSet t_packages; // TreeWidget items
	std::map<Private::Package*, Private::PackageItem*> pi_by_p;

	for (Private::Package& p: _model->packages())
		m_packages.insert (&p);

	for (int i = 0; i < _tree->invisibleRootItem()->childCount(); ++i)
	{
		Private::PackageItem* pi = dynamic_cast<Private::PackageItem*> (_tree->invisibleRootItem()->child (i));
		if (!pi)
			continue;
		pi_by_p[pi->package()] = pi;
		t_packages.insert (pi->package());
	}

	PackagesSet added;
	PackagesSet removed;
	PackagesSet rest;
	std::set_difference (m_packages.begin(), m_packages.end(), t_packages.begin(), t_packages.end(), std::inserter (added, added.end()));
	std::set_difference (t_packages.begin(), t_packages.end(), m_packages.begin(), m_packages.end(), std::inserter (removed, removed.end()));
	std::set_intersection (m_packages.begin(), m_packages.end(), t_packages.begin(), t_packages.end(), std::inserter (rest, rest.end()));

	// Most safe is to remove items with removed packages first:
	for (Private::Package* p: removed)
		remove_package_item (pi_by_p[p]);
	for (Private::Package* p: added)
		create_package_item (p);
	for (Private::Package* p: rest)
		pi_by_p[p]->read();

	// Reselect selected item to update presets editor in case the selected item has been changed
	// in another PresetsManager instance:
	QTreeWidgetItem* selected_item = _tree->selected_item();
	if (selected_item)
	{
		_tree->clearSelection();
		selected_item->setSelected (true);
	}
}


void
PresetsManager::save_preset (Private::PresetItem* preset_item, bool with_patch)
{
	if (_saveable_unit)
	{
		if (with_patch)
			preset_item->preset()->save_state_of (_saveable_unit);
		_editor->save_preset (preset_item);
		try {
			_model->save_state();
		}
		catch (Exception const& e)
		{
			QMessageBox::warning (this, "Error", QString (e.what()).toHtmlEscaped());
		}
	}
}


Private::PackageItem*
PresetsManager::create_package_item (Private::Package* package)
{
	return new Private::PackageItem (_tree, package);
}


void
PresetsManager::remove_package_item (Private::PackageItem* package_item)
{
	_tree->invisibleRootItem()->removeChild (package_item);
	delete package_item;
}


std::string
PresetsManager::sanitize_urn (std::string urn) const
{
	for (auto& c: urn)
		if (c == '/')
			c = '_';
	return urn;
}

} // namespace Haruhi

