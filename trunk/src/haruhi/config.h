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

#ifndef HARUHI__CONFIG_H__INCLUDED
#define HARUHI__CONFIG_H__INCLUDED

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
#include <QtGui/QFont>
#include <QtGui/QPixmap>
#include <QtGui/QPixmapCache>
#include <QtXml/QDomNode>

// Haruhi:
#include <haruhi/utility/confusion.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/backtrace.h>


#ifndef HARUHI_SHARED_DIRECTORY
#define HARUHI_SHARED_DIRECTORY "share"
#endif


namespace Haruhi {

class Session;

} // namespace Haruhi


class Config
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
	class UnitConfiguration: public SaveableState
	{
	  public:
		typedef std::vector<FavoritePreset> FavoritePresets;

	  public:
		UnitConfiguration (QString const& urn);

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
		save() { Config::save_unit_configurations(); }

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
			element (Config::document.createElement ("event-hardware-template"))
		{ }

		EventHardwareTemplate (QString const& name):
			name (name),
			element (Config::document.createElement ("event-hardware-template"))
		{ }

		EventHardwareTemplate (QString const& name, QDomElement const& element):
			name (name),
			element (element)
		{ }

	  public:
		QString		name;
		QDomElement	element;
	};

	typedef std::vector<RecentSession>				RecentSessions;
	typedef std::list<EventHardwareTemplate>		EventHardwareTemplates;
	typedef std::map<QString, UnitConfiguration>	UnitConfigurations;

  private:
	// Prevent instantiating:
	Config() { }

	typedef std::map<QString, QString> StringsMap;

  public:
	/**
	 * Initializes Config.
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
	 * Returns UnitConfiguration by given URN. If configuration does not
	 * exist, creates one.
	 */
	static UnitConfiguration&
	unit_configuration (QString const& urn);

	/**
	 * Stores event hardware template on disk.
	 */
	static void
	save_event_hardware_templates() { save(); }

	/**
	 * Stores unit configurations on disk.
	 */
	static void
	save_unit_configurations() { save(); }

	template<class T>
		static inline T
		option (QString const& option_name);

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
	static const char*	xmlns;
	static QDomDocument	document;
	static int			spacing;
	static int			margin;
	static int			small_spacing;
	static int			window_margin;
	static int			dialog_margin;
	static QFont		small_font;
	static const char*	haruhi_dir_name;

#define HARUHI_CONFIG_HAS_ICON(key, file)					\
		static QPixmap key()								\
		{													\
			QPixmap p;										\
			if (!QPixmapCache::find (#key, p))				\
			{												\
				p = QPixmap (file);							\
				QPixmapCache::insert (#key, p);				\
			}												\
			return p;										\
		}

	struct Icons16
	{
#define HARUHI_CONFIG_HAS_ICON_16(key, file)				\
		HARUHI_CONFIG_HAS_ICON(key, HARUHI_SHARED_DIRECTORY"/images/16/"file)

		HARUHI_CONFIG_HAS_ICON_16 (all, "all.png");
		HARUHI_CONFIG_HAS_ICON_16 (add, "add.png");
		HARUHI_CONFIG_HAS_ICON_16 (rename, "rename.png");
		HARUHI_CONFIG_HAS_ICON_16 (remove, "remove.png");
		HARUHI_CONFIG_HAS_ICON_16 (configure, "configure.png");
		HARUHI_CONFIG_HAS_ICON_16 (colorpicker, "colorpicker.png");
		HARUHI_CONFIG_HAS_ICON_16 (template_, "template.png");
		HARUHI_CONFIG_HAS_ICON_16 (new_, "new.png");
		HARUHI_CONFIG_HAS_ICON_16 (ok, "ok.png");
		HARUHI_CONFIG_HAS_ICON_16 (keyboard, "keyboard.png");
		HARUHI_CONFIG_HAS_ICON_16 (session_manager, "session-manager.png");
		HARUHI_CONFIG_HAS_ICON_16 (insert, "insert.png");
		HARUHI_CONFIG_HAS_ICON_16 (load, "load.png");
		HARUHI_CONFIG_HAS_ICON_16 (save, "save.png");
		HARUHI_CONFIG_HAS_ICON_16 (save_as, "save_as.png");
		HARUHI_CONFIG_HAS_ICON_16 (exit, "exit.png");
		HARUHI_CONFIG_HAS_ICON_16 (connect, "connect.png");
		HARUHI_CONFIG_HAS_ICON_16 (disconnect, "disconnect.png");
		HARUHI_CONFIG_HAS_ICON_16 (unit, "unit.png");
		HARUHI_CONFIG_HAS_ICON_16 (port, "port.png");
		HARUHI_CONFIG_HAS_ICON_16 (port_group, "port-group.png");
		HARUHI_CONFIG_HAS_ICON_16 (audio_input_port, "audio-input-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (audio_output_port, "audio-output-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (event_input_port, "event-input-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (event_output_port, "event-output-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (event_polyphonic_input_port, "event-polyphonic-input-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (event_polyphonic_output_port, "event-polyphonic-output-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (input_unit, "input-unit.png");
		HARUHI_CONFIG_HAS_ICON_16 (output_unit, "output-unit.png");
		HARUHI_CONFIG_HAS_ICON_16 (input_unit_bay, "input-unit-bay.png");
		HARUHI_CONFIG_HAS_ICON_16 (output_unit_bay, "output-unit-bay.png");
		HARUHI_CONFIG_HAS_ICON_16 (show_program, "show-program.png");
		HARUHI_CONFIG_HAS_ICON_16 (show_audio, "show-audio.png");
		HARUHI_CONFIG_HAS_ICON_16 (show_event, "show-event.png");
		HARUHI_CONFIG_HAS_ICON_16 (show_backends, "show-backends.png");
		HARUHI_CONFIG_HAS_ICON_16 (presets_package, "presets-package.png");
		HARUHI_CONFIG_HAS_ICON_16 (presets_category, "presets-category.png");
		HARUHI_CONFIG_HAS_ICON_16 (preset, "preset.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_sine, "wave-sine.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_triangle, "wave-triangle.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_square, "wave-square.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_sawtooth, "wave-sawtooth.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_pulse, "wave-pulse.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_power, "wave-power.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_gauss, "wave-gauss.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_diode, "wave-diode.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_chirp, "wave-chirp.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_random_square, "wave-random-square.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_random_triangle, "wave-random-triangle.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_noise, "wave-noise.png");
		HARUHI_CONFIG_HAS_ICON_16 (modulator_ring, "modulator-ring.png");
		HARUHI_CONFIG_HAS_ICON_16 (modulator_fm, "modulator-fm.png");
		HARUHI_CONFIG_HAS_ICON_16 (adsr, "adsr.png");
		HARUHI_CONFIG_HAS_ICON_16 (lfo, "lfo.png");

#undef HARUHI_CONFIG_HAS_ICON_16
	};

	struct Icons22
	{
#define HARUHI_CONFIG_HAS_ICON_22(key, file)				\
		HARUHI_CONFIG_HAS_ICON(key, HARUHI_SHARED_DIRECTORY"/images/22/"file)

		HARUHI_CONFIG_HAS_ICON_22 (spacer, "spacer.png");
		HARUHI_CONFIG_HAS_ICON_22 (show_audio, "show-audio.png");
		HARUHI_CONFIG_HAS_ICON_22 (show_event, "show-event.png");
		HARUHI_CONFIG_HAS_ICON_22 (connections, "connections.png");
		HARUHI_CONFIG_HAS_ICON_22 (configure, "configure.png");

#undef HARUHI_CONFIG_HAS_ICON_22
	};

#undef HARUHI_CONFIG_HAS_ICON

  private:
	static QString					_home;
	static QString					_xdg_config_home;
	static QString					_xdg_data_home;
	static QString					_file_name;
	static QString					_application_name;
	static StringsMap				_map;
	static RecentSessions			_recent_sessions;
	static EventHardwareTemplates	_event_hardware_templates;
	static UnitConfigurations		_unit_configurations;
};

#endif

#include "config.tcc"

