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

#ifndef HARUHI__SETTINGS__SETTINGS_H__INCLUDED
#define HARUHI__SETTINGS__SETTINGS_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <utility>
#include <iostream>

// Qt:
#include <QtCore/QFile>
#include <QtXml/QDomNode>

// Haruhi:
#include <haruhi/utility/confusion.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/backtrace.h>


class Settings
{
  public:
	/**
	 * Represents favorited preset.
	 */
	class FavoritePreset
	{
	  public:
		FavoritePreset (QString const& uuid, QString const& name):
			uuid (uuid),
			name (name)
		{ }

		bool
		operator< (FavoritePreset const& other) const
		{
			return uuid < other.uuid;
		}

		static bool
		eq_by_uuid (FavoritePreset const& a, FavoritePreset const& b)
		{
			return a.uuid == b.uuid;
		}

	  public:
		QString	uuid;
		QString	name;
	};

	/**
	 * Represents configuration for Unit.
	 */
	class UnitSettings: public SaveableState
	{
	  public:
		typedef std::vector<FavoritePreset> FavoritePresets;

	  public:
		UnitSettings (QString const& urn);

		QString const&
		urn() const { return _urn; }

		FavoritePresets&
		favorite_presets() { return _favorite_presets; }

		FavoritePresets const&
		favorite_presets() const { return _favorite_presets; }

		void
		uniq_favorite_presets();

		void
		save_state (QDomElement&) const;

		void
		load_state (QDomElement const&);

		QDomElement&
		config_element() { return _config_element; }

		void
		save() { Settings::save_unit_settings_list(); }

	  private:
		QString			_urn;
		FavoritePresets	_favorite_presets;
		QDomElement		_config_element;
	};

	/**
	 * Represents entry in a list of recent sessions
	 * shown in session manager.
	 */
	class RecentSession
	{
	  public:
		RecentSession():
			name(),
			file_name(),
			timestamp (0)
		{ }

		RecentSession (QString name, QString file_name, int timestamp):
			name (name),
			file_name (file_name),
			timestamp (timestamp)
		{ }

		/**
		 * Predicates used by standard algorithms.
		 */

		static bool
		gt_by_timestamp (RecentSession const& a, RecentSession const& b)
		{
			return a.timestamp > b.timestamp;
		}

		static bool
		lt_by_name (RecentSession const& a, RecentSession const& b)
		{
			return a.name < b.name;
		}

		static bool
		lt_by_file_name_and_gt_by_timestamp (RecentSession const& a, RecentSession const& b)
		{
			return std::make_pair (a.file_name, -a.timestamp) < std::make_pair (b.file_name, -b.timestamp);
		}

		static bool
		eq_by_file_name (RecentSession const& a, RecentSession const& b)
		{
			return a.file_name == b.file_name;
		}

		static bool
		file_not_exist (RecentSession const& a)
		{
			return !QFile::exists (a.file_name);
		}

	  public:
		QString		name;
		QString		file_name;
		int			timestamp;
	};

	/**
	 * Represents event hardware template that can be
	 * saved/inserted into event backend inputs list.
	 */
	class EventHardwareTemplate
	{
	  public:
		EventHardwareTemplate():
			name(),
			element (Settings::document.createElement ("event-hardware-template"))
		{ }

		EventHardwareTemplate (QString const& name):
			name (name),
			element (Settings::document.createElement ("event-hardware-template"))
		{ }

		EventHardwareTemplate (QString const& name, QDomElement const& element):
			name (name),
			element (element)
		{ }

	  public:
		QString		name;
		QDomElement	element;
	};

	typedef std::vector<RecentSession>			RecentSessions;
	typedef std::list<EventHardwareTemplate>	EventHardwareTemplates;
	typedef std::map<QString, UnitSettings>		UnitSettingsList;

  private:
	// Prevent instantiating:
	Settings() { }

	typedef std::map<QString, QString> StringsMap;

  public:
	/**
	 * Initializes Settings.
	 *
	 * \param	application_name
	 * 			Unique application name. On UNIX configuration will be stored
	 * 			in "~/.application_name".
	 */
	static void
	initialize (QString const& application_name);

	/**
	 * Should be called before program ends.
	 * Saves configuration.
	 */
	static void
	deinitialize();

	/**
	 * Returns list of RecentSessions.
	 */
	static RecentSessions&
	recent_sessions() { return _recent_sessions; }

	/**
	 * Removes duplicates and sessions with nonexistent files, limits number of recent sessions.
	 * Operates on internal list.
	 */
	static void
	update_recent_sessions();

	/**
	 * Returns list of EventHardwareTemplates.
	 */
	static EventHardwareTemplates&
	event_hardware_templates() { return _event_hardware_templates; }

	/**
	 * Returns UnitSettings by given URN. If configuration does not
	 * exist, creates one.
	 */
	static UnitSettings&
	unit_settings (QString const& urn);

	/**
	 * Stores event hardware template on disk.
	 */
	static void
	save_event_hardware_templates() { save(); }

	/**
	 * Stores unit configurations on disk.
	 */
	static void
	save_unit_settings_list() { save(); }

	/**
	 * Returns user's home directory.
	 */
	static QString
	home() { return _home; }

	/**
	 * Returns directory for config files.
	 */
	static QString
	config_home() { return _xdg_config_home; }

	/**
	 * Returns directory for shared data/presets/whatever.
	 */
	static QString
	data_home() { return _xdg_data_home; }

  private:
	static void
	load();

	static void
	save();

  public:
	/**
	 * Here are preloaded config options.
	 */
	static QDomDocument	document;
	static const char*	haruhi_dir_name;

  private:
	static QString					_home;
	static QString					_xdg_config_home;
	static QString					_xdg_data_home;
	static QString					_file_name;
	static QString					_application_name;
	static StringsMap				_map;
	static RecentSessions			_recent_sessions;
	static EventHardwareTemplates	_event_hardware_templates;
	static UnitSettingsList			_unit_settings_list;
};

#endif

