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
#include <cstdlib>
#include <cstdio>
#include <string>
#include <fstream>
#include <algorithm>
#include <iterator>

// System:
#include <libgen.h>
#include <string.h>
#include <errno.h>

// Qt:
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QPixmapCache>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/exception.h>
#include <haruhi/utility/filesystem.h>
#include <haruhi/utility/qdom.h>

// Local:
#include "settings.h"


namespace Haruhi {

Settings::Module::Module (QString const& name):
	_name (name)
{
}


void
Settings::Module::save()
{
	Mutex::Lock lock (*_host);
	_host->save();
}


Settings::Settings (QString const& settings_file, XDGHome xdg_home, QString const& template_file):
	_settings_file (settings_file),
	_template_file (template_file)
{
	switch (xdg_home)
	{
		case XDG_NONE:
			// Path is absolute, do nothing.
			break;

		case XDG_CONFIG:
			if (_settings_file.mid (0, 1) != "/")
				_settings_file.prepend ("/");
			_settings_file.prepend (xdg_config_home());
			break;

		case XDG_DATA:
			if (_settings_file.mid (0, 1) != "/")
				_settings_file.prepend ("/");
			_settings_file.prepend (xdg_data_home());
			break;
	}
}


void
Settings::register_module (Module* module)
{
	if (module->_host)
		module->_host->unregister_module (module);

	Modules::iterator m = _modules.find (module->name());
	if (m != _modules.end())
	{
		std::cerr << "Warning: settings module with name '" << module->name().toStdString() << "' already registered. Overwriting." << std::endl;
		unregister_module (m->second);
	}
	_modules[module->name()] = module;
	module->_host = this;

	// Immediately call module->load_state() if settings are already loaded:
	ModuleElements::iterator e = _module_elements.find (module->name());
	if (e != _module_elements.end())
		module->load_state (e->second);
}


void
Settings::unregister_module (Module* module)
{
	_modules.erase (module->name());
	module->_host = nullptr;
}


void
Settings::load()
{
	clear_document();

	// Load settings file:
	QFile file (_settings_file);

	if (!file.exists())
	{
		if (!_template_file.isEmpty())
		{
			create_dirs();
			QFile (_template_file).copy (_settings_file);
		}
		else
			return;
	}

	if (!file.open (QFile::ReadOnly))
		std::cerr << "Warning: failed to open settings file." << std::endl;
	else if (!_document.setContent (&file, true))
		std::cerr << "Warning: failed to parse settings file." << std::endl;

	file.close();
	parse();
}


void
Settings::save()
{
	create_dirs();
	clear_document();

	QDomElement root = _document.createElement ("haruhi-settings");
	_document.appendChild (root);

	// Collect settings from modules:
	for (auto& m: _modules)
	{
		QDomElement e = _document.createElement ("module");
		m.second->save_state (e);
		e.setAttribute ("name", m.first);
		root.appendChild (e);
	}

	// Save XML file:
	QFile file (_settings_file + "~");
	if (!file.open (QFile::WriteOnly))
		throw Exception ((QString ("could not save settings file: ") + file.errorString()).toUtf8());
	QTextStream ts (&file);
	ts << _document.toString();
	file.flush();
	file.close();
	if (::rename ((_settings_file + "~").toUtf8(), _settings_file.toUtf8()) == -1)
	{
		char buf[256];
		strerror_r (errno, buf, std::size (buf));
		throw Exception (QString ("could not save settings file: %1").arg (buf).toUtf8());
	}
}


void
Settings::create_dirs()
{
	// Create required directories:
	char* copy = strdup (_settings_file.toUtf8());
	char* dir = dirname (copy);
	mkpath (dir, 0700);
	free (copy);
}


void
Settings::parse()
{
	if (_document.documentElement().tagName() != "haruhi-settings")
	{
		std::cerr << "Warning: failed to process settings file." << std::endl;
		return;
	}

	for (QDomElement& e: _document.documentElement())
	{
		// Load module settings:
		if (e.tagName() == "module" && !e.attribute ("name").isEmpty())
		{
			QString name = e.attribute ("name");
			_module_elements[name] = e;
			// Call load_state() on appropriate module:
			Modules::iterator m = _modules.find (name);
			if (m != _modules.end())
				m->second->load_state (_module_elements[name]);
		}
	}
}


QString
Settings::home()
{
	char const* str = getenv ("HOME");
	return str ? str : "/";
}


QString
Settings::xdg_config_home()
{
	char const* str = getenv ("XDG_CONFIG_HOME");
	return str ? QString (str) : home() + "/.config";
}


QString
Settings::xdg_data_home()
{
	char const* str = getenv ("XDG_DATA_HOME");
	return str ? QString (str) : home() + "/.local/share";
}


void
Settings::clear_document()
{
	QDomNode n;
	while (!(n = _document.firstChild()).isNull())
		_document.removeChild (n);
}

} // namespace Haruhi
