/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__APPLICATION__HARUHI_H__INCLUDED
#define HARUHI__APPLICATION__HARUHI_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// System:
#include <unistd.h>

// Qt:
#include <QtCore/QObject>
#include <QtGui/QApplication>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/components/devices_manager/settings.h>
#include <haruhi/settings/settings.h>
#include <haruhi/settings/haruhi_settings.h>
#include <haruhi/settings/has_presets_settings.h>
#include <haruhi/settings/session_loader_settings.h>
#include <haruhi/utility/noncopyable.h>
#include <haruhi/utility/backtrace.h>


namespace Haruhi {

class UnitManager;
class Session;
class PeriodicUpdater;

/**
 * Haruhi/system - stores shared data,
 * contains main Session object, Settings
 * objects, etc. Created by main().
 */
class Haruhi:
	public QObject,
	private Noncopyable
{
	Q_OBJECT

  public:
	Haruhi (int argc, char** argv, char** envp);

	~Haruhi();

	void
	run_ui();

	/**
	 * Return Qt Application object.
	 */
	QApplication*
	application();

	/**
	 * Return global Haruhi singleton.
	 */
	static Haruhi*
	haruhi();

	/**
	 * Return global Session object.
	 */
	Session*
	session();

	/**
	 * Return global Settings object.
	 */
	Settings*
	settings() const;

	HaruhiSettings*
	haruhi_settings() const;

	DevicesManager::Settings*
	devices_manager_settings() const;

	HasPresetsSettings*
	has_presets_settings() const;

	SessionLoaderSettings*
	session_loader_settings() const;

  public slots:
	/**
	 * Show modal session loader dialog.
	 */
	void
	session_loader();

	/**
	 * Tell that when last window is closed
	 * application should quit.
	 */
	void
	ok_to_quit();

	/**
	 * Called when last window is closed.
	 * Calls QApplication::quit().
	 */
	void
	quit_if_ok();

  private:
	// Haruhi pointer:
	static Haruhi*				_haruhi;

	// Links to main application components:
	QApplication*				_app;
	PeriodicUpdater*			_periodic_updater;
	Session*					_session;
	Settings*					_settings;
	HaruhiSettings*				_haruhi_settings;
	DevicesManager::Settings*	_devices_manager_settings;
	HasPresetsSettings*			_has_presets_settings;
	SessionLoaderSettings*		_session_loader_settings;
	bool						_ok_to_quit;

	// Other:
	int		_argc;
	char**	_argv;
	char**	_envp;
};


static inline Haruhi*
g_haruhi()
{
	return Haruhi::haruhi();
}


inline QApplication*
Haruhi::application()
{
	return _app;
}


inline Haruhi*
Haruhi::haruhi()
{
	return _haruhi;
}


inline Session*
Haruhi::session()
{
	return _session;
}


inline Settings*
Haruhi::settings() const
{
	return _settings;
}


inline HaruhiSettings*
Haruhi::haruhi_settings() const
{
	return _haruhi_settings;
}


inline DevicesManager::Settings*
Haruhi::devices_manager_settings() const
{
	return _devices_manager_settings;
}


inline HasPresetsSettings*
Haruhi::has_presets_settings() const
{
	return _has_presets_settings;
}


inline SessionLoaderSettings*
Haruhi::session_loader_settings() const
{
	return _session_loader_settings;
}

} // namespace Haruhi

#endif

