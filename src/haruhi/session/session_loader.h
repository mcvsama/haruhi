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

#ifndef HARUHI__SESSION__SESSION_LOADER_H__INCLUDED
#define HARUHI__SESSION__SESSION_LOADER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QWidget>
#include <QDialog>
#include <QTabWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QComboBox>
#include <QListWidget>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/components/devices_manager/device.h>
#include <haruhi/settings/session_loader_settings.h>
#include <haruhi/widgets/generic_item.h>


namespace Haruhi {

class Session;


class SessionLoader: public QDialog
{
	Q_OBJECT

	class RecentSessionItem: public QTreeWidgetItem
	{
	  public:
		RecentSessionItem (QTreeWidget* parent, SessionLoaderSettings::RecentSession const& recent_session);

		void
		setup();

	  public:
		SessionLoaderSettings::RecentSession recent_session;
	};

	class DeviceItem: public QListWidgetItem
	{
	  public:
		DeviceItem (QListWidget* parent, DevicesManager::Device device);

		void setup();

	  public:
		DevicesManager::Device device;
	};

  public:
	enum RejectButton { CancelButton, QuitButton };
	enum DefaultTab { NewTab, OpenTab, AutoTab };
	enum Result { NoResult, NewSession, OpenSession };

  public:
	/**
	 * Creates SessionLoader dialog.
	 * \param	reject_button Text that should be on dialog rejection button.
	 * \param	default_tab Tab that should be visible by default.
	 * \param	parent Parent widget.
	 */
	SessionLoader (DefaultTab default_tab, RejectButton reject_button, QWidget* parent);

	// Dtor
	~SessionLoader();

	/**
	 * Applies configuration to the Session (creates audio ports,
	 * inserts event devices, sets name for the session, etc.)
	 */
	void
	apply (Session*);

  private slots:
	void
	update_widgets();

	void
	validate_and_accept();

	void
	browse_file();

	void
	open_recent (QTreeWidgetItem*, int);

	/**
	 * Reads DevicesManagerSettings and creates entries
	 * in _devices_combobox with items that are not
	 * already added to _devices_list.
	 */
	void
	populate_devices_combo();

	/**
	 * Auto add devices marked as auto-add to _devices_list.
	 */
	void
	auto_add_devices();

	/**
	 * Adds device currently selected in _devices_combobox to _devices_list.
	 * Calls add_device (Device);
	 */
	void
	add_selected_device();

	/**
	 * Adds given Device to _devices_list.
	 */
	void
	add_device (DevicesManager::Device const& device);

	/**
	 * Removes devuce currently selected in _devices_list.
	 * Does nothing if nothing is selected.
	 */
	void
	del_selected_device();

  private:
	Unique<QTabWidget>		_tabs;
	Unique<QWidget>			_new_tab;
	Unique<QWidget>			_open_tab;
	Unique<QPushButton>		_load_from_file_button;
	Unique<QPushButton>		_open_button;
	Unique<QPushButton>		_quit_button;
	Unique<QLineEdit>		_new_session_name;
	Unique<QSpinBox>		_new_session_audio_inputs;
	Unique<QSpinBox>		_new_session_audio_outputs;
	Unique<QPushButton>		_devices_add;
	Unique<QPushButton>		_devices_del;
	Unique<QComboBox>		_devices_combobox;
	Unique<QListWidget>		_devices_list;
	Unique<QTreeWidget>		_recent_listview;
	Result					_result;
	QString					_file_name;
};


inline
SessionLoader::RecentSessionItem::RecentSessionItem (QTreeWidget* parent, SessionLoaderSettings::RecentSession const& recent_session):
	QTreeWidgetItem (parent, QStringList() << recent_session.name << QString ("%1").arg (::basename (recent_session.file_name.toUtf8()))),
	recent_session (recent_session)
{
	setup();
}


inline void
SessionLoader::RecentSessionItem::setup()
{
	make_standard_height (this);
}


inline
SessionLoader::DeviceItem::DeviceItem (QListWidget* parent, DevicesManager::Device device):
	QListWidgetItem (Resources::Icons16::keyboard(), device.name(), parent),
	device (device)
{
	setup();
}


inline void
SessionLoader::DeviceItem::setup()
{
	make_standard_height (this);
}

} // namespace Haruhi

#endif

