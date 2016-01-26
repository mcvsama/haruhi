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
#include <string>

// Qt:
#include <QTextCodec>
#include <QApplication>
#include <QStackedWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QLayout>
#include <QPixmapCache>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/session/session.h>
#include <haruhi/session/session_loader.h>
#include <haruhi/session/periodic_updater.h>

// Local:
#include "haruhi.h"
#include "services.h"


namespace Haruhi {

Haruhi* Haruhi::_haruhi = 0;


Haruhi::Haruhi (int argc, char** argv, char** envp):
	_session (0),
	_ok_to_quit (false),
	_argc (argc),
	_argv (argv),
	_envp (envp)
{
	if (_haruhi)
		throw Exception ("only one instance of Haruhi is allowed");

	_haruhi = this;

	Services::initialize();

	QPixmapCache::setCacheLimit (2048); // 2MB cache

	_settings = new Settings (HARUHI_XDG_SETTINGS_HOME "/haruhi.conf", Settings::XDG_CONFIG,
							  HARUHI_SHARED_DIRECTORY "/config/haruhi.conf");

	_haruhi_settings = new HaruhiSettings();
	_devices_manager_settings = new DevicesManager::Settings();
	_has_presets_settings = new HasPresetsSettings();
	_session_loader_settings = new SessionLoaderSettings();

	_settings->register_module (_haruhi_settings);
	_settings->register_module (_devices_manager_settings);
	_settings->register_module (_has_presets_settings);
	_settings->register_module (_session_loader_settings);

	_settings->load();

	this->run_ui();

	_settings->save();

	_settings->unregister_module (_has_presets_settings);
	_settings->unregister_module (_session_loader_settings);
	_settings->unregister_module (_devices_manager_settings);
	_settings->unregister_module (_haruhi_settings);

	delete _session_loader_settings;
	delete _has_presets_settings;
	delete _devices_manager_settings;
	delete _haruhi_settings;
	delete _settings;
}


Haruhi::~Haruhi()
{
	Services::deinitialize();

	_haruhi = 0;

	QPixmapCache::clear();
}


void
Haruhi::run_ui()
{
	_app = new QApplication (_argc, _argv);
	// Fix "disabled" color for some styles to be gray instead of black:
	_app->setPalette (fix_palette (_app->palette()));
	// Now casting QString to std::string|const char* will yield UTF-8 encoded strings.
	// Also std::strings and const chars* are expected to be encoded in UTF-8.

	_periodic_updater = new PeriodicUpdater (30);
	QObject::connect (_app, SIGNAL (lastWindowClosed()), this, SLOT (quit_if_ok()));
	session_loader();
	if (_session)
		_app->exec();
	// Close current session:
	delete _session;
	delete _periodic_updater;
	delete _app;
}


void
Haruhi::session_loader()
{
	SessionLoader* loader = new SessionLoader (SessionLoader::AutoTab, _session ? SessionLoader::CancelButton : SessionLoader::QuitButton, 0);
	if (loader->exec() == SessionLoader::Accepted)
	{
		if (_session)
			_session->deleteLater();
		_session = new Session (0);
		_session->resize (600, 500);
		_session->show();
		loader->apply (_session);
	}
	delete loader;
}


void
Haruhi::ok_to_quit()
{
	_ok_to_quit = true;
}


void
Haruhi::quit_if_ok()
{
	if (_ok_to_quit)
		_app->quit();
}


QPalette
Haruhi::fix_palette (QPalette palette)
{
	auto blend = [](QColor input) {
		input.setAlpha (0x7f);
		return input;
	};

	palette.setColor (QPalette::Disabled, QPalette::WindowText, blend (palette.color (QPalette::Normal, QPalette::WindowText)));
	palette.setColor (QPalette::Disabled, QPalette::Text, blend (palette.color (QPalette::Normal, QPalette::Text)));
	palette.setColor (QPalette::Disabled, QPalette::ButtonText, blend (palette.color (QPalette::Normal, QPalette::ButtonText)));
	return palette;
}

} // namespace Haruhi

