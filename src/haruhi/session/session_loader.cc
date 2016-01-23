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
#include <QtGui/QWidget>
#include <QtGui/QDialog>
#include <QtGui/QLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QListWidget>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QFileDialog>
#include <QtGui/QTreeWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QGroupBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/session/session.h>
#include <haruhi/settings/session_loader_settings.h>

// Local:
#include "session_loader.h"


namespace Haruhi {

using namespace ScreenLiterals;


SessionLoader::SessionLoader (DefaultTab default_tab, RejectButton reject_button, QWidget* parent):
	QDialog (parent),
	_result (NoResult)
{
	setWindowTitle ("Haruhi — Session Control");
	setWindowIcon (QPixmap ("share/images/haruhi.png"));
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	_tabs = new QTabWidget (this);
	_tabs->setTabPosition (QTabWidget::North);

	//
	// Open session
	//

	_open_tab = new QWidget (_tabs);
	_tabs->addTab (_open_tab, "&Open session");

	_recent_listview = new QTreeWidget (_open_tab);
	_recent_listview->header()->hide();
	_recent_listview->header()->setResizeMode (QHeaderView::Stretch);
	_recent_listview->setAllColumnsShowFocus (true);
	_recent_listview->setMinimumSize (90_screen_mm, 90_screen_mm);
	_recent_listview->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_recent_listview->setHeaderLabel ("Session name");
	_recent_listview->setSelectionMode (QTreeWidget::SingleSelection);
	_recent_listview->setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
	_recent_listview->setRootIsDecorated (false);
	_recent_listview->setColumnCount (2);
	QObject::connect (_recent_listview, SIGNAL (itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT (open_recent (QTreeWidgetItem*, int)));

	QVBoxLayout* open_layout = new QVBoxLayout (_open_tab);
	open_layout->setSpacing (Config::spacing());
	open_layout->setMargin (2 * Config::margin());
	open_layout->addItem (new QSpacerItem (0, Config::spacing(), QSizePolicy::Fixed, QSizePolicy::Fixed));
	open_layout->addWidget (new QLabel ("Recent sessions:", _open_tab));
	open_layout->addWidget (_recent_listview);

	// Populate recent_listview:
	SessionLoaderSettings* settings = Haruhi::haruhi()->session_loader_settings();
	for (auto& rs: settings->recent_sessions())
		new RecentSessionItem (_recent_listview, rs);

	if (_recent_listview->invisibleRootItem()->childCount() > 0)
		_recent_listview->setCurrentItem (_recent_listview->invisibleRootItem()->child (0));

	//
	// New session
	//

	_new_tab = new QWidget (_tabs);
	_tabs->addTab (_new_tab, "&New session");

	_new_session_name = new QLineEdit ("New session", _new_tab);
	_new_session_name->selectAll();
	_new_session_name->setFocus();

	QGroupBox* name_box = new QGroupBox ("Session name", _new_tab);
	QGroupBox* audio_box = new QGroupBox ("Audio setup", _new_tab);
	QGroupBox* event_box = new QGroupBox ("Devices setup", _new_tab);

	_new_session_audio_inputs = new QSpinBox (audio_box);
	_new_session_audio_inputs->setMinimum (0);
	_new_session_audio_inputs->setMaximum (16);
	_new_session_audio_inputs->setValue (2);

	_new_session_audio_outputs = new QSpinBox (audio_box);
	_new_session_audio_outputs->setMinimum (0);
	_new_session_audio_outputs->setMaximum (16);
	_new_session_audio_outputs->setValue (2);

	_devices_combobox = new QComboBox (event_box);
	_devices_combobox->setIconSize (Resources::Icons16::haruhi().size());
	_devices_combobox->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	_devices_add = new QPushButton (Resources::Icons16::add(), "", audio_box);
	_devices_add->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_devices_add->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_devices_add, SIGNAL (clicked()), this, SLOT (add_selected_device()));

	_devices_del = new QPushButton (Resources::Icons16::remove(), "", audio_box);
	_devices_del->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_devices_del->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_devices_del, SIGNAL (clicked()), this, SLOT (del_selected_device()));

	_devices_list = new QListWidget (audio_box);

	QGridLayout* new_grid = new QGridLayout (name_box);
	new_grid->setSpacing (Config::spacing());
	new_grid->addWidget (_new_session_name, 0, 0);

	QGridLayout* audio_grid = new QGridLayout (audio_box);
	audio_grid->setSpacing (Config::spacing());
	audio_grid->setMargin (3 * Config::margin());
	audio_grid->setHorizontalSpacing (100);
	audio_grid->addWidget (new QLabel ("Audio inputs:", _new_tab), 0, 0);
	audio_grid->addWidget (_new_session_audio_inputs, 0, 1);
	audio_grid->addWidget (new QLabel ("Audio outputs:", _new_tab), 1, 0);
	audio_grid->addWidget (_new_session_audio_outputs, 1, 1);

	QGridLayout* event_grid = new QGridLayout (event_box);
	event_grid->setSpacing (Config::spacing());
	event_grid->setMargin (2 * Config::margin());
	event_grid->addWidget (_devices_combobox, 0, 0);
	event_grid->addWidget (_devices_add, 0, 1);
	event_grid->addWidget (_devices_del, 0, 2);
	event_grid->addWidget (_devices_list, 1, 0, 1, 3);

	QVBoxLayout* new_layout = new QVBoxLayout (_new_tab);
	new_layout->setSpacing (Config::spacing());
	new_layout->setMargin (3 * Config::margin());
	new_layout->addWidget (name_box);
	new_layout->addWidget (audio_box);
	new_layout->addWidget (event_box);

	//
	// Buttons
	//

	_load_from_file_button = new QPushButton (Resources::Icons16::open(), "&Load from file…", this);
	_load_from_file_button->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_load_from_file_button, SIGNAL (clicked()), this, SLOT (browse_file()));

	_open_button = new QPushButton (Resources::Icons16::new_(), "Open", this);
	_open_button->setIconSize (Resources::Icons16::haruhi().size());
	_open_button->setDefault (true);
	QObject::connect (_open_button, SIGNAL (clicked()), this, SLOT (validate_and_accept()));

	_quit_button = new QPushButton (Resources::Icons16::exit(), reject_button == CancelButton ? "Cancel" : "Quit", this);
	_quit_button->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_quit_button, SIGNAL (clicked()), this, SLOT (reject()));

	QObject::connect (_tabs, SIGNAL (currentChanged (QWidget*)), this, SLOT (update_widgets()));

	switch (default_tab)
	{
		case OpenTab:
			_tabs->setCurrentWidget (_open_tab);
			break;

		case NewTab:
			_tabs->setCurrentWidget (_new_tab);
			break;

		case AutoTab:
			if (_recent_listview->invisibleRootItem()->childCount() > 0)
				_tabs->setCurrentWidget (_open_tab);
			else
				_tabs->setCurrentWidget (_new_tab);
			break;
	}

	auto_add_devices();

	//
	// Layout
	//

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::spacing());
	buttons_layout->addWidget (_load_from_file_button);
	buttons_layout->addItem (new QSpacerItem (0, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (_open_button);
	buttons_layout->addWidget (_quit_button);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::dialog_margin());
	layout->setSpacing (Config::spacing());
	layout->addWidget (_tabs);
	layout->addLayout (buttons_layout);

	populate_devices_combo();
	_devices_list->clearSelection();
	_recent_listview->setFocus();
	update_widgets();
}


void
SessionLoader::apply (Session* session)
{
	switch (_result)
	{
		case NoResult:
			break;

		case NewSession:
		{
			session->set_name (_new_session_name->text());
			// Create audio inputs/outputs:
			for (int i = 0, n = _new_session_audio_inputs->value(); i < n; ++i)
				session->graph()->audio_backend()->create_input (QString ("in %1").arg (i + 1));
			for (int i = 0, n = _new_session_audio_outputs->value(); i < n; ++i)
				session->graph()->audio_backend()->create_output (QString ("out %1").arg (i + 1));
			// Add event devices:
			EventBackendImpl::Backend* backend = dynamic_cast<EventBackendImpl::Backend*> (session->graph()->event_backend());
			if (backend)
			{
				for (int i = 0; i < _devices_list->count(); ++i)
				{
					DeviceItem* device_item = dynamic_cast<DeviceItem*> (_devices_list->item (i));
					assert (device_item);
					backend->add_device (device_item->device);
				}
			}
			break;
		}

		case OpenSession:
			session->load_session (_file_name);
			break;
	}
}


void
SessionLoader::update_widgets()
{
	if (_tabs->currentWidget() == _new_tab)
	{
		_open_button->setText ("New");
		_open_button->setIcon (Resources::Icons16::new_());
	}
	else
	{
		_open_button->setText ("Open");
		_open_button->setIcon (Resources::Icons16::open());
	}
}


void
SessionLoader::validate_and_accept()
{
	if (_tabs->currentWidget() == _new_tab)
	{
		_result = NewSession;
		accept();
	}
	else if (_tabs->currentWidget() == _open_tab)
	{
		_result = OpenSession;
		if (!_recent_listview->selectedItems().empty())
			open_recent (_recent_listview->selectedItems().front(), 0);
	}
}


void
SessionLoader::browse_file()
{
	QFileDialog* file_dialog = new QFileDialog (this, "Load session", ".", QString());
	file_dialog->setFileMode (QFileDialog::ExistingFile);
	file_dialog->setNameFilter ("All Haruhi session files (*.haruhi-session)");
	file_dialog->setAcceptMode (QFileDialog::AcceptOpen);
	if (file_dialog->exec() == QFileDialog::Accepted)
	{
		_result = OpenSession;
		_file_name = file_dialog->selectedFiles().front();
		accept();
	}
}


void
SessionLoader::open_recent (QTreeWidgetItem* item, int)
{
	RecentSessionItem* recent_session_item = dynamic_cast<RecentSessionItem*> (item);
	if (recent_session_item)
	{
		_result = OpenSession;
		_file_name = recent_session_item->recent_session.file_name;
		accept();
	}
}


void
SessionLoader::populate_devices_combo()
{
	_devices_combobox->clear();

	DevicesManager::Settings* dm_settings = Haruhi::haruhi()->devices_manager_settings();
	assert (dm_settings);
	DevicesManager::Model::Devices& devices = dm_settings->model().devices();

	for (DevicesManager::Device& d: devices)
	{
		bool already_added = false;
		for (int i = 0; i < _devices_list->count(); ++i)
		{
			DeviceItem* ld = dynamic_cast<DeviceItem*> (_devices_list->item (i));
			if (!ld)
				continue;
			if (d == ld->device)
			{
				already_added = true;
				break;
			}
		}
		if (already_added)
			continue;
		_devices_combobox->addItem (Resources::Icons16::keyboard(), d.name(), qVariantFromValue (reinterpret_cast<void*> (&d)));
	}

	_devices_combobox->setEnabled (_devices_combobox->count() > 0);
	_devices_add->setEnabled (_devices_combobox->count() > 0);
	_devices_del->setEnabled (_devices_list->count() > 0);
}


void
SessionLoader::auto_add_devices()
{
	DevicesManager::Settings* dm_settings = Haruhi::haruhi()->devices_manager_settings();
	assert (dm_settings);
	DevicesManager::Model::Devices& devices = dm_settings->model().devices();

	for (DevicesManager::Device& d: devices)
		if (d.auto_add())
			add_device (d);
}


void
SessionLoader::add_selected_device()
{
	QVariant var = _devices_combobox->itemData (_devices_combobox->currentIndex());
	add_device (*reinterpret_cast<DevicesManager::Device*> (var.value<void*>()));
}


void
SessionLoader::add_device (DevicesManager::Device const& device)
{
	DeviceItem* device_item = new DeviceItem (_devices_list, device);
	_devices_list->addItem (device_item);
	_devices_list->setCurrentItem (device_item);
	device_item->setSelected (true);
	populate_devices_combo();
}


void
SessionLoader::del_selected_device()
{
	QList<QListWidgetItem*> selected_items = _devices_list->selectedItems();
	if (!selected_items.empty())
	{
		QListWidgetItem* item = selected_items.first();
		_devices_list->takeItem (_devices_list->row (item));
		delete item;
	}
	populate_devices_combo();
}

} // namespace Haruhi

