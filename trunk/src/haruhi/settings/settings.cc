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
#include <cstdlib>
#include <cstdio>
#include <string>
#include <fstream>
#include <algorithm>
#include <iterator>

// Qt:
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QApplication>
#include <QtGui/QPixmapCache>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/exception.h>
#include <haruhi/utility/filesystem.h>

// Local:
#include "settings.h"


/**
 * Settings statics
 */

QDomDocument						Settings::document;
const char*							Settings::haruhi_dir_name = "haruhi";

QString								Settings::_home;
QString								Settings::_xdg_config_home;
QString								Settings::_xdg_data_home;
QString								Settings::_file_name;
QString								Settings::_application_name;
Settings::StringsMap  				Settings::_map;
Settings::RecentSessions			Settings::_recent_sessions;
Settings::EventHardwareTemplates	Settings::_event_hardware_templates;
Settings::UnitSettingsList			Settings::_unit_settings_list;


Settings::UnitSettings::UnitSettings (QString const& urn):
	_urn (urn)
{
	_config_element = Settings::document.createElement ("config");
}


void
Settings::UnitSettings::uniq_favorite_presets()
{
	std::sort (favorite_presets().begin(), favorite_presets().end());
	FavoritePresets::iterator e = std::unique (favorite_presets().begin(), favorite_presets().end(), FavoritePreset::eq_by_uuid);
	favorite_presets().erase (e, favorite_presets().end());
}


void
Settings::UnitSettings::save_state (QDomElement& element) const
{
	QDomDocument document = element.ownerDocument();

	//
	// Favorite presets:
	//

	QDomElement favorite_presets_element = document.createElement ("favorite-presets");
	for (FavoritePresets::const_iterator f = favorite_presets().begin(); f != favorite_presets().end(); ++f)
	{
		QDomElement fp = document.createElement ("preset");
		fp.setAttribute ("uuid", f->uuid);
		fp.setAttribute ("name", f->name);
		favorite_presets_element.appendChild (fp);
	}
	element.appendChild (favorite_presets_element);

	//
	// Settings
	//

	element.appendChild (_config_element);
}


void
Settings::UnitSettings::load_state (QDomElement const& element)
{
	favorite_presets().clear();
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (e.tagName() == "favorite-presets")
		{
			for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
			{
				QDomElement e = n.toElement();
				if (!e.isNull() && e.tagName() == "preset")
					favorite_presets().push_back (Settings::FavoritePreset (e.attribute ("uuid"), e.attribute ("name")));
			}
		}
		else if (e.tagName() == "config")
			_config_element = e;
	}
}


void
Settings::initialize (QString const& application_name)
{
	QPixmapCache::setCacheLimit (2048); // 2MB cache

	_application_name = application_name;
	_file_name = _application_name + ".conf";

	Settings::load();
}


void
Settings::deinitialize()
{
	Settings::save();
}


void
Settings::load()
{
	Settings::_recent_sessions.clear();

	//
	// Settings directories
	//

	char const* HOME = getenv ("HOME");
	char const* XDG_CONFIG_HOME = getenv ("XDG_CONFIG_HOME");
	char const* XDG_DATA_HOME = getenv ("XDG_DATA_HOME");

	if (HOME)
		_home = HOME;
	else
	{
		std::clog << "Warning: HOME environment variable not defined, using '/'" << std::endl;
		_home = "/";
	}

	_xdg_config_home = (XDG_CONFIG_HOME ? QString (XDG_CONFIG_HOME) : _home + "/.config") + "/mulabs.org/" + haruhi_dir_name;
	_xdg_data_home = (XDG_DATA_HOME ? QString (XDG_DATA_HOME) : _home + "/.local/share") + "/mulabs.org/" + haruhi_dir_name;

	//
	// Create config dirs
	//

	mkpath (config_home().toStdString(), 0700);
	mkpath (data_home().toStdString(), 0700);

	// Load file:
	QFile file (config_home() + "/" + Settings::_file_name);
	if (!file.exists())
		QFile (HARUHI_SHARED_DIRECTORY "/config/haruhi.conf").copy (config_home() + "/" + Settings::_file_name);
	if (!file.open (IO_ReadOnly))
		std::cerr << "Warning: failed to open configuration file." << std::endl;
	else if (!Settings::document.setContent (&file, true))
		std::cerr << "Warning: failed to parse configuration file." << std::endl;
	file.close();

	// Process file:
	if (document.documentElement().tagName() == "haruhi-configuration")
	{
		for (QDomNode n = document.documentElement().firstChild(); !n.isNull(); n = n.nextSibling())
		{
			QDomElement e = n.toElement();
			if (!e.isNull())
			{
				if (e.tagName() == "recent-sessions")
				{
					for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
					{
						QDomElement e = n.toElement();
						if (!e.isNull())
						{
							if (e.tagName() == "recent-session")
								Settings::_recent_sessions.push_back (Settings::RecentSession (e.attribute ("name", "<Unknown name>"), e.attribute ("file-name", ""), e.attribute ("timestamp", "0").toInt()));
						}
					}
				}
				else if (e.tagName() == "event-hardware-templates")
				{
					for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
					{
						QDomElement e = n.toElement();
						if (!e.isNull())
						{
							if (e.tagName() == "event-hardware-template")
								Settings::_event_hardware_templates.push_back (Settings::EventHardwareTemplate (e.attribute ("name", "<Unknown name>"), e));
						}
					}
				}
				else if (e.tagName() == "unit-configurations")
				{
					for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
					{
						QDomElement e = n.toElement();
						if (!e.isNull() && e.tagName() == "unit-configuration")
						{
							QString unit_urn = e.attribute ("urn");
							if (!unit_urn.isNull())
								unit_settings (unit_urn).load_state (e);
						}
					}
				}
			}
		}
	}
	else
		std::cerr << "Warning: failed to process configuration file." << std::endl;

	update_recent_sessions();
}


void
Settings::save()
{
	// Create document:
	Settings::document = QDomDocument();
	QDomElement root = document.createElement ("haruhi-configuration");
	document.appendChild (root);

	//
	// Recent sessions
	//

	update_recent_sessions();

	QDomElement rss = document.createElement ("recent-sessions");
	for (RecentSessions::iterator s = recent_sessions().begin(); s != recent_sessions().end(); ++s)
	{
		QDomElement rs = document.createElement ("recent-session");
		rs.setAttribute ("name", s->name);
		rs.setAttribute ("file-name", s->file_name);
		rs.setAttribute ("timestamp", s->timestamp);
		rss.appendChild (rs);
	}
	root.appendChild (rss);

	//
	// Event hardware templates
	//

	QDomElement eht = document.createElement ("event-hardware-templates");
	for (EventHardwareTemplates::iterator t = event_hardware_templates().begin(); t != event_hardware_templates().end(); ++t)
	{
		t->element.setAttribute ("name", t->name);
		eht.appendChild (t->element);
	}
	root.appendChild (eht);

	//
	// Unit configurations:
	//

	QDomElement unit_settings_list_element = document.createElement ("unit-configurations");
	for (UnitSettingsList::iterator uc = _unit_settings_list.begin(); uc != _unit_settings_list.end(); ++uc)
	{
		QDomElement uc_element = document.createElement ("unit-configuration");
		uc->second.save_state (uc_element);
		uc_element.setAttribute ("urn", uc->second.urn());
		unit_settings_list_element.appendChild (uc_element);
	}
	root.appendChild (unit_settings_list_element);

	// Save file:
	QFile file (config_home() + "/" + Settings::_file_name + "~");
	if (!file.open (IO_WriteOnly))
		throw Exception (QString ("Could not save configuration file: ") + file.errorString());
	QTextStream ts (&file);
	ts << document.toString();
	file.flush();
	file.close();
	::rename ((config_home() + "/" + Settings::_file_name + "~").toUtf8(), (config_home() + "/" + Settings::_file_name).toUtf8());
}


void
Settings::update_recent_sessions()
{
	RecentSessions::iterator e;
	// Remove non-existent files:
	e = std::remove_if (_recent_sessions.begin(), _recent_sessions.end(), RecentSession::file_not_exist);
	_recent_sessions.erase (e, _recent_sessions.end());
	// Remove duplicates:
	std::sort (_recent_sessions.begin(), _recent_sessions.end(), RecentSession::lt_by_file_name_and_gt_by_timestamp);
	e = std::unique (_recent_sessions.begin(), _recent_sessions.end(), RecentSession::eq_by_file_name);
	_recent_sessions.erase (e, _recent_sessions.end());
	// Limit recent sessions entries:
	std::sort (_recent_sessions.begin(), _recent_sessions.end(), RecentSession::gt_by_timestamp);
	_recent_sessions.resize (std::min (static_cast<ptrdiff_t> (_recent_sessions.size()), static_cast<ptrdiff_t> (32)));
	std::sort (_recent_sessions.begin(), _recent_sessions.end(), RecentSession::lt_by_name);
}


Settings::UnitSettings&
Settings::unit_settings (QString const& urn)
{
	UnitSettingsList::iterator uc = _unit_settings_list.find (urn);
	if (uc != _unit_settings_list.end())
		return uc->second;
	else
		return _unit_settings_list.insert (std::make_pair (urn, UnitSettings (urn))).first->second;
}

