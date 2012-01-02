/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 * --
 * Configuration file format:
 *
 * <haruhi-settings>
 *  <module name="devices-manager">
 *    ...
 *  </module>
 *  <module name="session-loader">
 *    ...
 *  </module>
 * </haruhi-settings>
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
#include <haruhi/config/all.h>
#include <haruhi/utility/confusion.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/backtrace.h>
#include <haruhi/utility/mutex.h>


namespace Haruhi {

/**
 * Settings class reads and saves settings to file.
 * Provides API for Clients, which can access XML
 * subtrees and parse them.
 */
class Settings: public RecursiveMutex
{
  public:
	/**
	 * Settings client. When registered in Settings object,
	 * the latter will use SaveableState API to serialize/deserialize
	 * settings.
	 */
	class Module: public SaveableState
	{
		friend class Settings;

	  public:
		Module (QString const& name);

		/**
		 * Returns name that identifies module.
		 */
		QString
		name() const;

		/**
		 * Saves settings. Calls save on host Settings.
		 */
		virtual void
		save();

		/**
		 * Returns pointer to Settings object.
		 */
		Settings*
		host() const;

	  private:
		Settings*	_host;
		QString		_name;
	};

	enum XDGHome {
		XDG_NONE,	// Path to settings file is absolute.
		XDG_CONFIG,	// Path is relative to XDG config home.
		XDG_DATA	// Path is relative to XDG data home.
	};

  private:
	typedef std::map<QString, Module*> Modules;
	typedef std::map<QString, QDomElement> ModuleElements;

  public:
	/**
	 * Initializes settings object. Does not read settings, that
	 * is done by load().
	 *
	 * \param	settings_file
	 * 			Path to configuration file.
	 * \param	xdg_home
	 * 			Tells if settings_file is absolute path or relative to XDG
	 * 			defined directories for config/data files.
	 * \param	template_file
	 * 			If given and settings_file does not exist, it will be copied
	 * 			to settings_file place. If not given, settings XML document will
	 * 			be empty. This path must be absolute.
	 */
	Settings (QString const& settings_file, XDGHome xdg_home = XDG_NONE, QString const& template_file = QString());

	/**
	 * Registers module that will handle its own settings.
	 * Settings object does not take ownership of the module.
	 * Module must be unregistered before Settings object is deleted.
	 *
	 * If settings are already loaded when module is registered,
	 * Settings object immediately calls module->load_state()
	 * with appropriate XML element.
	 */
	void
	register_module (Module* module);

	/**
	 * Unregisters module.
	 */
	void
	unregister_module (Module* module);

	/**
	 * Reads settings.
	 */
	void
	load();

	/**
	 * Saves settings.
	 */
	void
	save();

	/**
	 * Returns directory prefix for configuration files.
	 */
	static QString
	config_home();

	/**
	 * Returns directory prefix for shared data files.
	 */
	static QString
	data_home();

	/**
	 * Returns QDomDocument that stores XML settings.
	 */
	QDomDocument&
	document();

	QDomDocument const&
	document() const;

  private:
	/**
	 * Creates dirs where file is to be saved.
	 */
	void
	create_dirs();

	/**
	 * Parses settings document, so Client classes can
	 * access their own sections.
	 */
	void
	parse();

	/**
	 * Returns user's home directory.
	 */
	static QString
	home();

	/**
	 * Returns XDG standarized directory for config (settings) files.
	 */
	static QString
	xdg_config_home();

	/**
	 * Returns XDG standarized directory for shared data/presets/whatever.
	 */
	static QString
	xdg_data_home();

	/**
	 * Removes all child nodes from document node.
	 */
	void
	clear_document();

  private:
	QString			_settings_file;
	QString			_template_file;
	QDomDocument	_document;
	Modules			_modules;
	ModuleElements	_module_elements;
};


inline QString
Settings::Module::name() const
{
	return _name;
}


inline Settings*
Settings::Module::host() const
{
	return _host;
}


inline QString
Settings::config_home()
{
	return xdg_config_home() + "/" + HARUHI_XDG_SETTINGS_HOME;
}


inline QString
Settings::data_home()
{
	return xdg_data_home() + "/" + HARUHI_XDG_DATA_HOME;
}


inline QDomDocument&
Settings::document()
{
	return _document;
}


inline QDomDocument const&
Settings::document() const
{
	return _document;
}

} // namespace Haruhi

#endif

