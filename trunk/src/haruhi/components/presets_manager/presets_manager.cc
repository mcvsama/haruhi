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
#include <cstring>
#include <cstdlib>
#include <algorithm>

// System:
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

// Qt:
#include <QtCore/QDir>
#include <QtXml/QDomNode>
#include <QtGui/QLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <Qt3Support/Q3ListView>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/unit.h>
#include <haruhi/utility/filesystem.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/memory.h>

// Local:
#include "preset_editor.h"
#include "package_item.h"
#include "category_item.h"
#include "preset_item.h"
#include "presets_list_view.h"
#include "presets_manager.h"


namespace Haruhi {

namespace Private = PresetsManagerPrivate;


PresetsManager::PresetsManager (Unit* unit, QWidget* parent):
	QWidget (parent),
	_unit (unit),
	_saveable_unit (dynamic_cast<SaveableState*> (unit)),
	_lock_file (-1)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QWidget* right_panel = new QWidget (this);
	right_panel->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

	_create_menu = new Q3PopupMenu (this);
	_create_menu->insertItem (Config::Icons16::presets_package(), "Package", this, SLOT (create_package()), 0, CreatePackage);
	_create_menu->insertItem (Config::Icons16::presets_category(), "Category", this, SLOT (create_category()), 0, CreateCategory);
	_create_menu->insertItem (Config::Icons16::preset(), "Preset", this, SLOT (create_preset()), 0, CreatePreset);

	_tabs = new QTabWidget (this);
	_tabs->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	QObject::connect (_tabs, SIGNAL (currentChanged (QWidget*)), this, SLOT (update_widgets()));

	_list = new Private::PresetsListView (this, this);
	QObject::connect (_list, SIGNAL (selectionChanged()), this, SLOT (update_widgets()));
	QObject::connect (_list, SIGNAL (doubleClicked (Q3ListViewItem*)), this, SLOT (load_preset (Q3ListViewItem*)));

	_favs = new Private::PresetsListView (this, this);
	QObject::connect (_favs, SIGNAL (selectionChanged()), this, SLOT (update_widgets()));

	_editor = new Private::PresetEditor (right_panel);

	_load_button = new QPushButton (Config::Icons16::load(), "Load", right_panel);
	QObject::connect (_load_button, SIGNAL (clicked()), this, SLOT (load_preset()));

	_save_button = new QPushButton (Config::Icons16::save(), "Save patch", right_panel);
	QObject::connect (_save_button, SIGNAL (clicked()), this, SLOT (save_preset()));

	_create_button = new QPushButton (Config::Icons16::save_as(), "Create", right_panel);
	_create_button->setPopup (_create_menu);

	_destroy_button = new QPushButton (Config::Icons16::remove(), "Destroy…", right_panel);
	QObject::connect (_destroy_button, SIGNAL (clicked()), this, SLOT (destroy()));

	_tabs->addTab (_list, "Presets");
	_tabs->addTab (_favs, "Favorities");

	// Layouts:

	QVBoxLayout* v1 = new QVBoxLayout (this, 0, Config::spacing);
	QHBoxLayout* h1 = new QHBoxLayout (v1, Config::spacing);
	h1->addWidget (_tabs);
	h1->addWidget (right_panel);
	QVBoxLayout* v2 = new QVBoxLayout (right_panel, 0, Config::spacing);
	QHBoxLayout* h2 = new QHBoxLayout (v2, Config::spacing);
	h2->addWidget (_load_button);
	h2->addWidget (_save_button);
	h2->addWidget (_create_button);
	h2->addWidget (_destroy_button);
	v2->addWidget (_editor);

	QString all_presets_dir = Config::data_home() + "/presets";
	mkpath (all_presets_dir.toStdString(), 0700);

	UnitFactory::InformationMap::const_iterator i = _unit->factory()->information().find ("haruhi:presets.directory");
	_packages_dir = "-";
	if (i != _unit->factory()->information().end())
	{
		_packages_dir = all_presets_dir + "/" + QString::fromStdString (i->second);
		read();
	}

	update_widgets();
}


PresetsManager::~PresetsManager()
{
	if (_lock_file != -1)
		::unlink (lock_file_name());
	delete _create_menu;
}


bool
PresetsManager::is_favorite (QString const& uuid)
{
	Config::UnitConfiguration& uc = Config::unit_configuration (QString::fromStdString (_unit->urn()));
	Config::UnitConfiguration::FavoritePresets& list = uc.favorite_presets();
	for (Config::UnitConfiguration::FavoritePresets::iterator f = list.begin(); f != list.end(); ++f)
		if (f->uuid == uuid)
			return true;
	return false;
}


void
PresetsManager::set_favorite (QString const& uuid, QString const& name, bool set)
{
	Config::UnitConfiguration& uc = Config::unit_configuration (QString::fromStdString (_unit->urn()));
	if (set)
		uc.favorite_presets().push_back (Config::FavoritePreset (uuid, name));
	else
	{
		Config::UnitConfiguration::FavoritePresets& list = uc.favorite_presets();
		for (Config::UnitConfiguration::FavoritePresets::iterator f = list.begin(); f != list.end(); )
		{
			if (f->uuid == uuid)
				f = list.erase (f);
			else
				++f;
		}
	}
	uc.uniq_favorite_presets();
	Config::save_unit_configurations();
}


void
PresetsManager::load_preset()
{
	if (_saveable_unit)
	{
		Private::PresetItem* preset_item = _list->current_preset_item();
		if (preset_item)
		{
			_saveable_unit->load_state (preset_item->patch());
			emit preset_selected (preset_item->uuid(), preset_item->meta().name);
		}
	}
}


void
PresetsManager::load_preset (Q3ListViewItem* item)
{
	item->listView()->setSelected (item, true);
	load_preset();
}


void
PresetsManager::save_preset()
{
	Private::PresetItem* preset_item = _list->current_preset_item();
	if (preset_item)
	{
		save_preset (preset_item, true);
		emit preset_selected (preset_item->uuid(), preset_item->meta().name);
	}
}


void
PresetsManager::create_package()
{
	Private::PackageItem* package_item = new Private::PackageItem (_list);
	package_item->meta().name = "<new package>";
	package_item->reload();
	_list->setSelected (package_item, true);
	_editor->focus_package();
}


void
PresetsManager::create_category()
{
	Private::PackageItem* package_item = _list->current_package_item();
	if (!package_item)
	{
		Private::CategoryItem* category_item = _list->current_category_item();
		if (category_item)
			package_item = category_item->package_item();
		else
		{
			Private::PresetItem* preset_item = _list->current_preset_item();
			if (preset_item)
				package_item = preset_item->category_item()->package_item();
		}
	}

	if (package_item)
	{
		Private::CategoryItem* category_item = new Private::CategoryItem ("<new category>", package_item);
		category_item->reload();
		_list->setSelected (category_item, true);
		_editor->focus_category();
		package_item->setOpen (true);
	}
}


void
PresetsManager::create_preset()
{
	Private::CategoryItem* category_item = _list->current_category_item();
	if (!category_item)
	{
		Private::PresetItem* preset_item = _list->current_preset_item();
		if (preset_item)
			category_item = preset_item->category_item();
	}

	if (category_item)
	{
		Private::PresetItem* preset_item = new Private::PresetItem (category_item);
		preset_item->meta().name = "<new preset>";
		preset_item->reload();
		_list->setSelected (preset_item, true);
		_editor->focus_name();
		category_item->setOpen (true);
		category_item->package_item()->setOpen (true);
	}
}


void
PresetsManager::destroy()
{
	Private::PackageItem* package_item = _list->current_package_item();
	if (package_item)
	{
		if (QMessageBox::question (this, "Delete package", "Really delete package " + package_item->meta().name + "?",
								   QMessageBox::Yes | QMessageBox::Default, QMessageBox::Cancel | QMessageBox::Escape) == QMessageBox::Yes)
		{
			package_item->remove_file();
			_list->takeItem (package_item);
			delete package_item;
		}
	}

	Private::CategoryItem* category_item = _list->current_category_item();
	if (category_item)
	{
		if (QMessageBox::question (this, "Delete category", "Really delete category " + category_item->name() + "?",
								   QMessageBox::Yes | QMessageBox::Default, QMessageBox::Cancel | QMessageBox::Escape) == QMessageBox::Yes)
		{
			Private::PackageItem* package_item = category_item->package_item();
			package_item->takeItem (category_item);
			delete category_item;
			try {
				package_item->save_file();
			}
			catch (Exception const& e)
			{
				QMessageBox::warning (this, "Error", e.what());
			}
		}
	}

	Private::PresetItem* preset_item = _list->current_preset_item();
	if (preset_item)
	{
		if (QMessageBox::question (this, "Delete preset", "Really delete preset " + preset_item->meta().name + "?",
								   QMessageBox::Yes | QMessageBox::Default, QMessageBox::Cancel | QMessageBox::Escape) == QMessageBox::Yes)
		{
			Private::CategoryItem* category_item = preset_item->category_item();
			category_item->takeItem (preset_item);
			delete preset_item;
			try {
				category_item->package_item()->save_file();
			}
			catch (Exception const& e)
			{
				QMessageBox::warning (this, "Error", e.what());
			}
		}
	}
}


QDomElement
PresetsManager::append_element (QDomElement& subject, QString const& name, QString const& value)
{
	QDomDocument doc = subject.ownerDocument();
	QDomElement element = doc.createElement (name);
	element.appendChild (doc.createTextNode (value));
	subject.appendChild (element);
	return element;
}


void
PresetsManager::update_widgets()
{
	// TODO który tab? czy package/grupa/patch? itp
	_load_button->setEnabled (_saveable_unit && _list->current_preset_item());
	_save_button->setEnabled (_saveable_unit && _list->current_preset_item());
	_create_button->setEnabled (_saveable_unit);
	_destroy_button->setEnabled (_saveable_unit);

	Private::PackageItem* package_item = _list->current_package_item();
	Private::CategoryItem* category_item = _list->current_category_item();
	Private::PresetItem* preset_item = _list->current_preset_item();

	_editor->setEnabled (true);

	if (package_item)
	{
		_editor->load_package (package_item);
		_create_menu->setItemEnabled (CreatePackage, true);
		_create_menu->setItemEnabled (CreateCategory, true);
		_create_menu->setItemEnabled (CreatePreset, false);
	}
	else if (category_item)
	{
		_editor->load_category (category_item);
		_create_menu->setItemEnabled (CreatePackage, true);
		_create_menu->setItemEnabled (CreateCategory, true);
		_create_menu->setItemEnabled (CreatePreset, true);
	}
	else if (preset_item)
	{
		_editor->load_preset (preset_item);
		_create_menu->setItemEnabled (CreatePackage, true);
		_create_menu->setItemEnabled (CreateCategory, true);
		_create_menu->setItemEnabled (CreatePreset, true);
	}
	else
	{
		_editor->setEnabled (false);
		_editor->clear();
		_create_menu->setItemEnabled (CreatePackage, true);
		_create_menu->setItemEnabled (CreateCategory, false);
		_create_menu->setItemEnabled (CreatePreset, false);
	}
}


void
PresetsManager::read()
{
	mkpath (directory().toStdString(), 0700);

	bool exit = false;
	// Try locking 5 times then give up:
	for (int i = 0; i < 5 && exit == false; ++i)
	{
		_lock_file = ::open (lock_file_name(), O_CREAT | O_EXCL | O_WRONLY, 0644);
		// If lock is acquired:
		if (_lock_file != -1)
		{
			// Write our pid to file:
			QString pid  = QString ("%1").arg (::getpid());
			if (::write (_lock_file, pid.ascii(), std::strlen (pid.ascii())) == -1)
				std::cerr << "Warning: failed to write Mikuru's PID to lock file." << std::endl;

			// Read packages:
			QDir dir (_packages_dir, "*.haruhi-presets");
			QStringList list = dir.entryList();
			// Each entry in list represents one presets package:
			for (QStringList::Iterator p = list.begin(); p != list.end(); ++p)
			{
				Private::PackageItem* package_item = new Private::PackageItem (_list);
				try {
					package_item->load_file (_packages_dir + "/" + *p);
				}
				catch (Exception const& e)
				{
					QMessageBox::warning (this, "Error", e.what());
				}
			}
			update_widgets();
			exit = true;
		}
		else
		{
			QFile f (lock_file_name());
			char pid_string[1024] = { 0, };
			if (f.open (IO_ReadOnly))
			{
				f.readLine (pid_string, 1023);
				int pid = std::atoi (pid_string);
				// Check if another process exist:
				if (::kill (pid, 0) == 0)
				{
					// If the other process is us, link two PresetsManagers to mirror their operations:
					// (first will be in master mode to operate on disk, second in slave to operate only in UI).
					if (::getpid() == pid)
					{
						// TODO
						QMessageBox::warning (this, "Presets Manager", "Another unit has locked the presets directory, sharing is unimplemented yet.");
						setEnabled (false);
						exit = true;
					}
					else
					{
						QMessageBox::warning (this, "Presets Manager", "Another process has locked presets directory, disabling Presets Manager.");
						setEnabled (false);
						exit = true;
					}
				}
				else
					// Does not exist, remove file and try again to read:
					f.remove();
			}
			// Couldn't open file, try to read again in a loop.
		}
	}

	if (exit == false)
	{
		QMessageBox::warning (this, "Presets Manager", "Could not read presets directory (could not acquire or clean lock).\nGiving up.");
		setEnabled (false);
	}
}


void
PresetsManager::save_preset (Private::PresetItem* preset_item, bool with_patch)
{
	if (_saveable_unit)
	{
		Private::PackageItem* package_item = preset_item->category_item()->package_item();
		if (with_patch)
		{
			preset_item->clear_patch_element (package_item->document());
			_saveable_unit->save_state (preset_item->patch());
		}
		_editor->save_preset (preset_item);
		try {
			package_item->save_file();
		}
		catch (Exception const& e)
		{
			QMessageBox::warning (this, "Error", e.what());
		}
	}
}


QString
PresetsManager::lock_file_name() const
{
	return _packages_dir + "/" + ".haruhi-lock.pid";
}

} // namespace Haruhi

